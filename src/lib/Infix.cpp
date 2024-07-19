#include <iostream>
#include <vector>
#include <deque>
#include <sstream>
#include <string>
#include <memory>

#include "Infix.hpp"
#include "Lexer.hpp"
#include "ASTNode.hpp"
#include "Environment.hpp"
#include "Exception.hpp"
#include "ValueSum.hpp"

std::unordered_map<std::string, int> PRECEDENCE = {
    {"(", 100}, {")", 100},
    {"*", 10}, {"/", 10}, {"%", 10},
    {"+", 9}, {"-", 9},
    {"<", 8}, {"<=", 8}, {">", 8}, {">=", 8},
    {"==", 7}, {"!=", 7},
    {"&", 6}, 
    {"^", 5},
    {"|", 4},
    {"=", 0}
};

std::unordered_map<std::string, std::string> ASSOCIATIVITY = {
    {"*", "left"}, {"/", "left"}, {"%", "left"},
    {"+", "left"}, {"-", "left"},
    {"<", "left"}, {"<=", "left"}, {">", "left"}, {">=", "left"},
    {"==", "left"}, {"!=", "left"},
    {"&", "left"}, 
    {"^", "left"},
    {"|", "left"},
    {"=", "right"}
};

Infix::Parser::Parser(std::deque<Token> input) { this->input = input; }

std::shared_ptr<ASTNode> Infix::Parser::parse(std::shared_ptr<Environment> env) {
    // Verify token sequence is legal
    if (input.size() == 1 && input[0].type == Type::END)
        throw UnexpectedToken(input[0], 1);
    
    int open = 0, operators = 0, operands = 0;
    for (size_t i = 0; i < input.size(); ++i) {
        Token token = input[i];
        if (token.type == Type::left_paren) open++;
        else if (token.type == Type::right_paren) open--;
        else if (token.type == Type::number || token.type == Type::identifier || token.type == Type::boolean) operands++;
        else if (token.type == Type::op || token.type == Type::assignment) operators++;
        
        if (i == input.size()-3 && token.type == Type::right_paren && operators == operands)
            throw UnexpectedToken(token);
        if (operators > operands)
            throw UnexpectedToken(token);
        if (open < 0)
            throw UnexpectedToken(token, 1);
        if (open > 0 && token.type == Type::END)
            throw UnexpectedToken(token, 1);
        if (open == 0 && token.type == Type::op)
            throw UnexpectedToken(token, 1);
    }
    if (operands == operators)
        throw UnexpectedToken(input.back(), 1);
    if (open > 0)
        throw UnexpectedToken(input.back(), 1);

    std::deque<Token> copy = input;
    return parse(copy, env);
}

std::shared_ptr<ASTNode> Infix::Parser::parse(std::deque<Token> &tokens, std::shared_ptr<Environment> env) {
    std::deque<Token> ops;
    std::deque<std::shared_ptr<ASTNode>> exps;

    // for (auto t : tokens) {
    //     std::cout << t.text << " ";
    // }
    // std::cout << std::endl;

    auto create_expr = [&](std::shared_ptr<Operator> op) {
        // Helper function to create a new expression
        // LEFT ASSOCIATIVE
        if (ASSOCIATIVITY[ops.front().text] == "left") {
            auto e1 = exps.front(); exps.pop_front();
            auto e2 = exps.front(); exps.pop_front();
            op->add_sub_expr(e2);
            op->add_sub_expr(e1);
            exps.push_front(op);
        }
        // Right ASSOCIATIVE
        else if (ASSOCIATIVITY[ops.front().text] == "right") {
            auto e1 = exps.front(); exps.pop_front();
            auto e2 = exps.front(); exps.pop_front();
            // if (ops.front().type == Type::assignment && e2->token.type != Type::identifier)
            //     throw UnexpectedToken(ops.front());
            op->add_sub_expr(e1);
            op->add_sub_expr(e2);
            exps.push_front(op);
        }
    };
    Token var;
    while (!tokens.empty()) {
        switch (tokens.front().type) {
            case Type::left_paren:
                ops.push_front(tokens.front());
                tokens.pop_front();
                break;
            case Type::right_paren:
                while (!ops.empty()) {
                    if (ops.front().type == Type::left_paren) {
                        ops.pop_front();
                        break;
                    }
                    if (exps.size() == 1) {
                        throw UnexpectedToken(exps.front()->token);
                    }
                    std::shared_ptr<Operator> op(new Operator(ops.front(), env));
                    create_expr(op);
                    ops.pop_front();
                }
                tokens.pop_front();
                break;
            case Type::null:
            case Type::boolean:
            case Type::number:
                exps.push_front(std::shared_ptr<Value>(new Value(tokens.front(), env)));
                tokens.pop_front();
                break;
            case Type::assignment:
                if (exps.empty())
                    throw UnexpectedToken(tokens.front());
                while (!ops.empty()) {
                    if (ops.front().type == Type::left_paren) break;
                    if (PRECEDENCE[ops.front().text] <= PRECEDENCE[tokens.front().text]) break;
                    std::shared_ptr<Operator> op(new Operator(ops.front(), env));
                    create_expr(op);
                    ops.pop_front();
                }
                ops.push_front(tokens.front());
                tokens.pop_front();
                break;
            case Type::op:
                if (exps.empty())
                    throw UnexpectedToken(tokens.front());
                while (!ops.empty()) {
                    if (ops.front().type == Type::left_paren) break;
                    if (PRECEDENCE[ops.front().text] < PRECEDENCE[tokens.front().text]) break;
                    if (exps.size() == 1)
                        throw UnexpectedToken(tokens.front());
                    std::shared_ptr<Operator> op(new Operator(ops.front(), env));
                    create_expr(op);
                    ops.pop_front();
                }
                ops.push_front(tokens.front());
                tokens.pop_front();
                break;
            case Type::identifier:
                var = tokens.front(); tokens.pop_front();
                // function call foo(...)
                if (tokens.front().type == Type::left_paren) {
                    // std::cout << "In function" << std::endl;
                    tokens.pop_front();
                    // We use var as a dummy token since it is not a statement
                    std::shared_ptr<Function> func(new Function(var, env, var.text, true));
                    while (tokens.front().type != Type::right_paren) {
                            std::deque<Token> call_expr_tokens;
                            int open = 0;
                            if (tokens.front().type == Type::comma)
                                throw UnexpectedToken(tokens.front());
                            while (tokens.front().type != Type::comma) {
                                if (tokens.front().type == Type::left_paren) open++;
                                else if (tokens.front().type == Type::right_paren) open--;
                                if (open < 0) break;
                                call_expr_tokens.push_back(tokens.front());
                                tokens.pop_front();
                            }
                            // std::cout << "call_expr_tokens: ";
                            // for (Token t : call_expr_tokens)
                            //     std::cout << t.text << " ";
                            // std::cout << std::endl;
                            if (!call_expr_tokens.empty()) {
                                call_expr_tokens.push_front(Token{"(", -1, -1, Type::left_paren});
                                call_expr_tokens.push_back(Token{")", -1, -1, Type::right_paren});
                                func->add_call_block(Infix::Parser(call_expr_tokens).parse(env));
                            }
                            if (tokens.front().type == Type::comma) tokens.pop_front();
                    }
                    exps.push_front(func);
                }
                // Its just a variable!
                else {
                    exps.push_front(std::shared_ptr<Identifier>(new Identifier(var, env)));
                }
                break;
            case Type::left_curly:
            case Type::right_curly:
            case Type::statement:
            case Type::comma:
            case Type::semi_colon:
                throw RuntimeError("Runetime Error: Not implemented.");
            case Type::END:
                tokens.pop_front();
                break;
        }
    }
    if (!ops.empty())
        throw UnexpectedToken(ops.front(), 1);
    if (exps.size() != 1)
        throw RuntimeError("Runetime Error: Expression stack should only have one remaining element");
    return exps.front();
};

std::pair<int, std::string> Infix::Parser::eval(std::shared_ptr<Environment> env) {
    // Builds AST and evaluates
    // Return <error code, value or error message>
    int code;
    std::string ret;
    auto copy_sp = std::make_shared<Environment>(Environment());
    try {
        copy_sp->clear();
        copy_sp->copy(env);
        std::shared_ptr<ASTNode> ast = parse(env);
        std::ostringstream stream;
        stream << vsum_to_string(ast->eval());
        ret = stream.str();
        code = 0;
    }
    catch(ScryptException& e) {
        env->clear();
        env->copy(copy_sp);
        code = e.code();
        ret = e.what();
    }
    catch(const std::runtime_error& e) {
        env->clear();
        env->copy(copy_sp);        
        code = 1;
        ret = e.what();
    }
    
    return std::make_pair(code, ret);
}