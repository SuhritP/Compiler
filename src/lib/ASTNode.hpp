#pragma once

#include <stdexcept>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <variant>
#include <sstream>

#include "Lexer.hpp"  // Needed for Token processing
#include "Environment.hpp"
#include "ValueSum.hpp"

/**
 * Base class for the nodes of an Abstract Syntax Tree (AST).
 * Each node in the tree represents a part of an expression,
 * which can be a number or an operator. Each node can be
 * converted to a string representation or can be evaluated to produce a result.
 */

extern std::ostringstream GLOBAL_COUT;

class ASTNode {
public:
    virtual std::string to_string() = 0;  // Convert node to string representation
    virtual ValueSum eval() = 0;
    virtual bool is_braced() = 0;
    Token token;  // Token associated with the node
    bool braced = false;
    // Stores the "scope" the node is in
    std::shared_ptr<Environment> env;
};  

// Represents a value in the AST.
class Value: public ASTNode {
public:
    Value(Token token, std::shared_ptr<Environment> env);

    std::string to_string();
    ValueSum eval() override;
    bool is_braced();

    ValueSum val;
};
class Identifier : public ASTNode {
public:
    Identifier(Token token, std::shared_ptr<Environment> env);
    std::string to_string();
    ValueSum eval() override;
    bool is_braced();
};
/**
 * Represents an operator (like +, -, *, /) in the AST.
 * An operator can have one or more operands (sub-expressions).
 */
class Operator: public ASTNode {
public:
    Operator(Token token, std::shared_ptr<Environment> env);

    std::string to_string();
    ValueSum eval() override;
    bool is_braced();

    // Add an operand for this operator
    void add_sub_expr(std::shared_ptr<ASTNode> expr);

// private:
    std::vector<std::shared_ptr<ASTNode>> sub_expr;  // Operands for this operator
};

class Block: public ASTNode {
public:
    Block(Token token, std::shared_ptr<Environment> env, bool braced = false);

    std::string to_string();
    ValueSum eval() override;

    void add_statement(std::shared_ptr<ASTNode> statement);
    void add_function(std::shared_ptr<Function> function);
    bool is_braced();

    std::vector<std::shared_ptr<ASTNode>> statements; 
    std::vector<std::shared_ptr<Function>> functions;
    size_t function_index = 0;
};

class Function : public ASTNode {
    public:
        Function(Token token, std::shared_ptr<Environment> env, std::string name, bool called = false);
    
        std::string to_string();
        ValueSum eval();
        bool is_braced();

        void add_func_block(std::shared_ptr<ASTNode> f_block);
        void add_call_block(std::shared_ptr<ASTNode> c_block);
        void add_arg_names(std::vector<std::string> arg_names);

    // private:
        std::weak_ptr<Environment> fenv_ptr;
        std::shared_ptr<ASTNode> func_block;
        std::vector<std::shared_ptr<ASTNode>> call_block;
        std::vector<std::string> arg_names;
        std::string name;
        bool called;
};