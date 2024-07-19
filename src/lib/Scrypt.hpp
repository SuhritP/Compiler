#pragma once

#include <iostream>
#include <vector>
#include <deque>
#include <unordered_map>
#include <string>
#include <memory>

#include "Lexer.hpp"
#include "ASTNode.hpp"
#include "Infix.hpp"
#include "Environment.hpp"

namespace Scrypt {

class Parser {
    public:
        std::shared_ptr<ASTNode> parse();
        std::shared_ptr<ASTNode> parse(std::deque<Token> &tokens, std::shared_ptr<Environment> env, std::string block_type = "__main__");

        Parser(std::deque<Token> input);
    
        std::deque<Token> input;
};

}