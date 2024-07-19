#pragma once

#include <iostream>
#include <vector>
#include <deque>
#include <unordered_map>
#include <string>
#include <memory>

#include "Lexer.hpp"
#include "ASTNode.hpp"
#include "Environment.hpp"

extern std::unordered_map<std::string, int> PRECEDENCE;
extern std::unordered_map<std::string, std::string> ASSOCIATIVITY;

namespace Infix {

class Parser {
    public:
        std::shared_ptr<ASTNode> parse(std::deque<Token> &tokens, std::shared_ptr<Environment> env);
        std::shared_ptr<ASTNode> parse(std::shared_ptr<Environment> env);
        Parser(std::deque<Token> input);
        std::pair<int, std::string> eval(std::shared_ptr<Environment> env);
        std::string to_string();

    // private:
        std::deque<Token> input;
};


}