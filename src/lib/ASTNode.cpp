#include <sstream>
#include <vector>
#include <functional>
#include <numeric>
#include <variant>
#include <cmath>

#include "ASTNode.hpp"
#include "Environment.hpp"
#include "Exception.hpp"
#include "ValueSum.hpp"

std::ostringstream GLOBAL_COUT;

// Constructor to initialize a value from a given token.
Value::Value(Token token, std::shared_ptr<Environment> env) { 
    this->token = token; 
    this->env = env;
    if (token.type == Type::number)
        val = ValueSum{std::stod(token.text)};
    else if (token.type == Type::boolean)
        val = ValueSum{(token.text == "true")};
    else if (token.type == Type::null)
        val = ValueSum{nullptr};
}

ValueSum Value::eval() { return val; }
// Converts the number to a string representation ensuring that there are no unnecessary trailing zeros.
std::string Value::to_string() { 
    return vsum_to_string(val);
}

std::string Identifier::to_string() { return token.text; }

Identifier::Identifier(Token token, std::shared_ptr<Environment> env) {
    this->token = token;
    this->env = env;
}

ValueSum Identifier::eval() {
    return env->get(token.text);
}

// Constructor to initialize an operator from a given token.
Operator::Operator(Token token, std::shared_ptr<Environment> env) { 
    this->token = token; 
    this->env = env;
}

// Converts the operator and its sub-expressions to a string representation in infix notation.
std::string Operator::to_string() {
    std::ostringstream oss;
    try {
        oss << "(";
        if (sub_expr.size() > 1) {
            if (token.text == "=") {
                oss << sub_expr.back()->to_string();
                for (unsigned int i = sub_expr.size() - 2; i < sub_expr.size(); --i) {
                    oss << " " << token.text << " ";
                    oss << sub_expr[i]->to_string();
                }
            }
            else {
                oss << sub_expr[0]->to_string();
                for (unsigned int i = 1; i < sub_expr.size(); ++i) {
                    oss << " " << token.text << " ";
                    oss << sub_expr[i]->to_string();
                }
            }
        }
        oss << ")";
    }
    catch(ScryptException& e) {
        // Handle exceptions that may occur during string conversion.
        std::cout << e.what() << std::endl;
        exit(e.code());
    }
    return oss.str();
}

// Evaluates the operator along with its sub-expressions and returns the result as a double.
ValueSum Operator::eval() {
    if (token.text == "=") {
        ValueSum value = sub_expr.front()->eval();
        for (size_t i = 1; i < sub_expr.size(); ++i) {
            if (sub_expr[i]->token.type != Type::identifier)
                throw RuntimeError("Runtime error: invalid assignee.");
                // throw UnexpectedToken(sub_expr[i]->token);
            env->add(sub_expr[i]->to_string(), value);
        }
        return value;
    }
    std::vector<ValueSum> values;
    for (auto sub: sub_expr) {
        values.push_back(sub->eval());
    }
    
    if (values.size() != 2)
        throw RuntimeError("Runetime Error: Illegal operation");
    ValueSum v1 = values[0], v2 = values[1];

    ValueSum ret;
    if (token.text == "+") 
        ret = ValueSum{get_number(v1) + get_number(v2)};
    else if (token.text == "-")
        ret = ValueSum{get_number(v1) - get_number(v2)};
    else if (token.text == "*")
        ret = ValueSum{get_number(v1) * get_number(v2)};
    else if (token.text == "/") {
        if (get_number(v2) == 0)
            throw RuntimeError("Runtime error: division by zero.");
        ret = ValueSum{get_number(v1) / get_number(v2)};
    }
    else if (token.text == "%") {
        if (get_number(v2) == 0)
            throw RuntimeError("Runtime error: division by zero.");
        ret = ValueSum{std::fmod(get_number(v1), get_number(v2))};
    }
    else if (token.text == "<")
        ret = ValueSum{get_number(v1) < get_number(v2)};
    else if (token.text == "<=") 
        ret = ValueSum{get_number(v1) <= get_number(v2)};
    else if (token.text == ">") 
        ret = ValueSum{get_number(v1) > get_number(v2)};
    else if (token.text == ">=") 
        ret = ValueSum{get_number(v1) >= get_number(v2)};
    else if (token.text == "==") {
        if (is_number(v1) && is_number(v2))
            ret = ValueSum{get_number(v1) == get_number(v2)};
        else if (is_bool(v1) && is_bool(v2))
            ret = ValueSum{get_bool(v1) == get_bool(v2)};
        else // comparing values of different type
            return ValueSum{false};
    }
    else if (token.text == "!=") {
        if (is_number(v1) && is_number(v2))
            ret = ValueSum{get_number(v1) != get_number(v2)};
        else if (is_bool(v1) && is_bool(v2))
            ret = ValueSum{get_bool(v1) != get_bool(v2)};
        else // comparing values of different type
            return ValueSum{false};
    }
    else if (token.text == "&")
        ret = ValueSum{get_bool(v1) && get_bool(v2)};
    else if (token.text == "^")
        ret = ValueSum{get_bool(v1) != get_bool(v2)}; // logical XOR is equivalent to !=
    else if (token.text == "|")
        ret = ValueSum{get_bool(v1) || get_bool(v2)};
    return ret;
}

// Adds a sub-expression to the list of sub-expressions for the operator.
void Operator::add_sub_expr(std::shared_ptr<ASTNode> expr) { 
    sub_expr.push_back(expr);
}

Block::Block(Token token, std::shared_ptr<Environment> env, bool braced) { 
    this->token = token;
    this->env = env;
    this->braced = braced; 
}
std::string Block::to_string() {
    std::ostringstream oss;
    std::shared_ptr<ASTNode> prev = nullptr;

    if (token.text != "__main__" && token.text != "__block__") {
        if (token.text == "else")
            oss << "\n";
        // Print statement token
        oss << token.text;
        if (!statements.empty() && statements[0]->to_string() != "__blank__")
            oss << " ";
    }
    for (size_t i = 0; i < statements.size(); ++i) {
        auto statement = statements[i];
        // expr -> (expr)
        if (statement->token.type != Type::statement) {
            if (statement->to_string() != "__blank__")
                oss << statement->to_string();
            if (token.text == "__main__" || token.text == "__block__" || token.text == "else" || token.text == "print" || token.text == "return")
                oss << ";";
        }
        else {
            if (statement->is_braced()) {
                // We are in some braced block
                std::string tab = "    "; // 4 spaces
                if (prev && prev->token.type != Type::statement)
                    oss << " ";
                oss << "{\n";

                // Add tabs
                std::string block_str = statement->to_string();

                oss << tab;
                for (size_t ci = 0; ci < block_str.length(); ++ci) {
                    oss << block_str[ci];
                    if (block_str[ci] == '\n' && ci != block_str.length() - 1) 
                        oss << tab;
                }
                oss << "\n}";
            }
            else {
                // Some expression
                oss << statement->to_string();
            }
        }
        prev = statement;
        if ((token.text == "__main__" || token.text == "__block__") && i != statements.size() - 1)
            oss << "\n";
    }
    return oss.str();
}
ValueSum Block::eval() {
    // Evaluate program AST, log prints into global outputstream
    
    if (token.text == "__main__" || token.text == "__block__" || token.text == "else") {
        // We are in some kind of braced block
        // Evaluate all the statements within
        for (auto statement : statements) {
            // std::cout << statement->to_string() << std::endl;
            // std::cout << "block env:\n" <<  statement->env->to_string() << std::endl;
            if (statement->token.text == "def") {
                // statement is a function
                if (!functions.empty() && function_index < functions.size()) {
                    env->add(functions[function_index]->name, ValueSum{functions[function_index]});
                    auto lock = functions[function_index]->fenv_ptr.lock();
                    if (lock){
                        lock->copy(env);
                    }
                    function_index++;
                }
            }
            statement->eval();
        }
    }
    else {
        if (token.text == "if") {
            auto cond = statements[0];
            auto body = statements[1];

            if (!is_bool(cond->eval()))
                throw RuntimeError("Runtime error: condition is not a bool.");
            if (get_bool(cond->eval()))
                body->eval();
            else if (statements.size() == 3)
                statements[2]->eval();  // Evaluate else body
        }
        else if (token.text == "while") {
            auto cond = statements[0];
            auto body = statements[1];

            // this eval might have side effects?
            if (!is_bool(cond->eval()))
                throw RuntimeError("Runtime error: condition is not a bool.");
            
            while (get_bool(cond->eval())) {
                body->eval();
                if (!is_bool(cond->eval()))
                    throw RuntimeError("Runtime error: condition is not a bool.");
            }
        }
        else if (token.text == "print") {
            auto body = statements[0];
            GLOBAL_COUT << vsum_to_string(body->eval()) << '\n';
        }
        else if (token.text == "return") {
            // return; -> nullptr
            // std::cout << "RETURN called: ";
            // std::cout << statements.size() << std::endl;
            // std::cout << "ENV: " << env->to_string() << std::endl;
            if (statements.empty()) throw ReturnThrow(ValueSum{nullptr});
            if (statements[0]->token.text == "__blank__") throw ReturnThrow(ValueSum{nullptr});
            // return expr; -> eval(expr)
            // std::cout << "RETURN called: ";
            // std::cout << vsum_to_string(statements[0]->eval()) << std::endl;
            throw ReturnThrow(statements[0]->eval());
        }
    }
    return ValueSum{false};
}
void Block::add_statement(std::shared_ptr<ASTNode> statement) {
    statements.push_back(statement);
}
void Block::add_function(std::shared_ptr<Function> function) {
    functions.push_back(function);
}

bool Block::is_braced() { return braced; }

bool Value::is_braced()         { return braced; }
bool Identifier::is_braced()    { return braced; }
bool Operator::is_braced()      { return braced; }
bool Function::is_braced()      { return braced; }

Function::Function(Token token, std::shared_ptr<Environment> env, std::string name, bool called) {
    this->token = token;
    this->fenv_ptr = env;
    this->name = name;
    this->called = called;
}

std::string Function::to_string() {
    std::ostringstream oss;
    // Function decleration
    // def foo(...) {...}
    // std::cout << "Func to_string called" << std::endl;
    if (!called) {
        // def foo
        // std::cout << "start Function tostring signature" << std::endl;
        oss << "def" << " " << name;
        // (x, y, ..., z)
        oss << "(";
        // std::cout << "arg_names " << arg_names.size() << std::endl;
        for (size_t i = 0; i < arg_names.size() - 1 && !arg_names.empty(); ++i) {
            oss << arg_names[i] << ", ";
        }
        if (arg_names.size() >= 1) oss << arg_names.back();
        oss << ") ";
        // Print block
        oss << "{\n";
        if (func_block->to_string().empty())
            oss << "}";
        else {
            std::string tab = "    "; // 4 spaces
            // std::cout << "finish Function tostring signature" << std::endl;
            std::string block_str = func_block->to_string();
            oss << tab;
            for (size_t ci = 0; ci < block_str.length(); ++ci) {
                oss << block_str[ci];
                if (block_str[ci] == '\n' && ci != block_str.length() - 1) 
                    oss << tab;
            }
            oss << "\n}";
        }
    }
    // function called: foo(...)
    else {
        oss << name << "(";
        for (size_t i = 0; i < call_block.size() - 1  && !call_block.empty(); ++i) {
            oss << call_block[i]->to_string() << ", ";
        }
        if (call_block.size() >= 1) oss << call_block.back()->to_string();
        oss << ")";
    }
    return oss.str();
}


void Function::add_func_block(std::shared_ptr<ASTNode> f_block) {
    this->func_block = f_block;
}

void Function::add_call_block(std::shared_ptr<ASTNode> c_block) {
    this->call_block.push_back(c_block);
}

void Function::add_arg_names(std::vector<std::string> arg_names) {
    this->arg_names = arg_names;
}

ValueSum Function::eval() {
    // Capture closure if function defined
    // if (!called) {
    //     // std::cout << "ENV before capture:\n" << env->to_string() << std::endl;
    //     if (!env->parent) throw RuntimeError("Function has no parent env");
    //     // env->copy(env->parent);
    //     // env->parent->add(name, std::shared_ptr<Function>(this));
    //     std::cout << "ENV after capture:\n" << env->to_string() << std::endl;
        
    // }

    //  If called, evaluate and get return value
    if (called) {
        try {
            // std::cout << "Function called " << name << std::endl;
            // std::cout << "ENV: " << env->to_string() << std::endl;
            // Find function definition in environment and add call_block
            std::shared_ptr<Function> func;
            auto lock = fenv_ptr.lock();
            if (lock){
                if (lock->contains(name) && is_function(lock->get(name))) { 
                    func = get_function(lock->get(name));
                }
                else {
                    throw RuntimeError("Runtime error: not a function.");
                }
                func->call_block.clear();
                for (auto c_block : call_block)  
                    func->add_call_block(c_block);
                // std::cout << "Call blocks added" << std::endl;
                if (func->call_block.size() != func->arg_names.size())
                    throw RuntimeError("Runtime error: incorrect argument count.");
                // Assign arguments with values
                // This overwrites previous ones
                for (size_t i = 0; i < func->arg_names.size(); ++i) {
                    auto lock2 = func->fenv_ptr.lock();
                    if (lock2)
                        lock2->add(func->arg_names[i], func->call_block[i]->eval());
                }

                func->func_block->eval();
                // std::cout << "Function evaluated" << std::endl;
            }
        }
        catch (ReturnThrow& ret) {
            // std::cout << "Return value: " << vsum_to_string(ret.get_ret()) << std::endl;
            return ret.get_ret();
        }
        // Default return
        return ValueSum{nullptr};
    }
    return ValueSum{false};
}