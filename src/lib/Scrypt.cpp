#include <iostream>
#include <vector>
#include <deque>
#include <unordered_map>
#include <string>
#include <memory>
#include <iomanip>

#include "Lexer.hpp"
#include "ASTNode.hpp"
#include "Infix.hpp"
#include "Scrypt.hpp"
#include "Exception.hpp"
#include "Environment.hpp"

Scrypt::Parser::Parser(std::deque<Token> input) { this->input = input; }
std::shared_ptr<ASTNode> Scrypt::Parser::parse() {
    if (input.size() == 1 && input[0].type == Type::END)
        throw UnexpectedToken(input[0], 1);
    // Check if parentheses are balanced
    int open_paren = 0, open_curly = 0;
    for (size_t i = 0; i < input.size(); ++i) {
        Token token = input[i];
        if (token.type == Type::left_paren) open_paren++;
        else if (token.type == Type::right_paren) open_paren--;
        else if (token.type == Type::left_curly) open_curly++;
        else if (token.type == Type::right_curly) open_curly--;

        if (open_curly < 0 || open_paren < 0)
            throw UnexpectedToken(token);
    }
    // Create our global environment
    auto sp = std::shared_ptr<Environment>(new Environment());
    return parse(input, sp);
}
std::shared_ptr<ASTNode> Scrypt::Parser::parse(std::deque<Token> &tokens, std::shared_ptr<Environment> env, std::string block_type) {
    std::shared_ptr<Block> block(new Block(Token{block_type, -1, -1, Type::statement}, env, block_type == "__block__"));

    auto get_next_expr = [&](size_t &pos){
        // Return the next expression from tokens, starting at index pos
        // fixes pos at the next token after the expression
        // Greedily take tokens until a statement, curly or next line
        // size_t start = pos;
        std::deque<Token> expr;
        for ( ; pos < tokens.size(); ++pos) {
            // check valid type
            // expression could terminate at semicolon, statement, left_curly
            if (tokens[pos].type == Type::semi_colon || tokens[pos].type == Type::statement || tokens[pos].type == Type::left_curly)
                break;
            // // check same position as start
            // if (tokens[pos].line_number != tokens[start].line_number)
            //     break;
            expr.push_back(tokens[pos]);
        }
        // std::cout << "Expression: " << std::endl;
        // for (Token token : expr) {
        //     std::cout << std::right << std::setw(4) << token.line_number 
        //             << std::setw(5) << token.column_number 
        //             << "  " << token.text << std::endl;
        // }
        if (expr.empty()) {
            std::shared_ptr<ASTNode> ret(new Identifier(Token{"__blank__", -1, -1, Type::identifier}, env));
            return ret;
        }
        expr.push_front(Token{"(", -1, -1, Type::left_paren});
        expr.push_back(Token{")", -1, -1, Type::right_paren});
        return Infix::Parser(expr).parse(expr, env);
        // }
        // return ret;
    };

    auto get_next_block = [&](size_t &pos, std::shared_ptr<Environment> block_env){
        // Return the next block from tokens, starting at index pos
        // fixes pos at the next token after the expression
        // Expects pos to be the index of the first curly
        std::deque<Token> braced_block;
        int unclosed_curly = 0;
        do {
            if (tokens[pos].type == Type::left_curly) unclosed_curly++;
            if (tokens[pos].type == Type::right_curly) unclosed_curly--;
            braced_block.push_back(tokens[pos]);
            ++pos;
        } while (pos < tokens.size() && unclosed_curly > 0);
        // delete front and back curly braces
        braced_block.pop_front();
        braced_block.pop_back();
        // std::cout << "Block: " << std::endl;
        // for (Token token : braced_block) {
        //     std::cout << std::right << std::setw(4) << token.line_number 
        //             << std::setw(5) << token.column_number 
        //             << "  " << token.text << std::endl;
        // }
        // Empty block added
        if (braced_block.empty()) {
            std::shared_ptr<ASTNode> ret(new Block(Token{block_type, -1, -1, Type::statement}, env, block_type == "__block__"));
            return ret;
        }
        return parse(braced_block, block_env, "__block__");
    };

    for (size_t i = 0; i < tokens.size();) {
        Token token = tokens[i];
        if (token.type == Type::END) break;
        else if (token.type == Type::statement) {
            if (token.text == "if") {
                // Create a branch block
                std::shared_ptr<Block> branch_block(new Block(token, env));
                // we do not expect a semicolon after condition in if
                // if cond {}
                std::shared_ptr<ASTNode> cond        = get_next_expr(++i);
                std::shared_ptr<ASTNode> braced      = get_next_block(i, env);
                branch_block->add_statement(cond);
                branch_block->add_statement(braced);
                std::shared_ptr<Block> curr = branch_block;
                // Optionally create a false branch block
                // We will keep creating these blocks until weve exhausted all "else if" tokens
                while (tokens[i].text == "else") {
                    std::shared_ptr<Block> else_block(new Block(tokens[i], env));
                    if (tokens[++i].text == "if") {
                        std::shared_ptr<Block> new_branch_block(new Block(tokens[i], env, "__block__"));
                        std::shared_ptr<ASTNode> new_cond        = get_next_expr(++i);
                        std::shared_ptr<ASTNode> new_braced      = get_next_block(i, env);
                        new_branch_block->add_statement(new_cond);
                        new_branch_block->add_statement(new_braced);     
                        else_block->add_statement(new_branch_block);
                        curr->add_statement(else_block);
                        curr = new_branch_block;
                    }
                    else {
                        // This is the final else block
                        else_block->add_statement(get_next_block(i, env));
                        curr->add_statement(else_block);
                    }
                }
                // Add the root of the if/else if/else tree to main block
                block->add_statement(branch_block);
            }
            else if (token.text == "while") {
                std::shared_ptr<Block>   while_block(new Block(token, env));
                std::shared_ptr<ASTNode> cond        = get_next_expr(++i);
                std::shared_ptr<ASTNode> braced      = get_next_block(i, env);
                while_block->add_statement(cond);
                while_block->add_statement(braced);
                block->add_statement(while_block);
            }
            else if (token.text == "print" || token.text == "return") {
                std::shared_ptr<Block>   print_block(new Block(token, env));
                // We expect semicolon after expression in print
                // print expr;
                std::shared_ptr<ASTNode> expr = get_next_expr(++i);
                if (tokens[i++].type != Type::semi_colon)
                    throw UnexpectedToken(tokens[i-1]);
                print_block->add_statement(expr);
                block->add_statement(print_block);
            }
            else if (token.text == "def") {
                // std::cout << "Def statement found" << std::endl;
                auto closure = std::shared_ptr<Environment>(new Environment());
                // closure->set_parent(env);
                Token func_name = tokens[++i];
                // match all arguments
                // (x, y, ... , z)
                std::vector<std::string> arg_names = {};
                // std::cout << "start parsing arg_names" << std::endl;
                if (tokens[++i].type != Type::left_paren) throw UnexpectedToken(tokens[i-1]);
                if (tokens[i+1].type == Type::right_paren) {
                    i += 2;
                }
                else {
                    while (tokens[i].type != Type::right_paren) {
                        // get arg name
                        arg_names.push_back(tokens[++i].text);
                        // we now expect either a "," or ")"
                        if (tokens[++i].type == Type::comma) continue;
                        else if (tokens[i].type == Type::right_paren) {
                            i++; // Point i to next token after closing ")"
                            break;
                        }
                    }
                }
                // std::cout << "arg_names parsed" << std::endl;
                std::shared_ptr<Function> func(new Function(token, closure, func_name.text));
                // std::cout << "Adding func block " << i << std::endl;
                func->add_func_block(get_next_block(i, closure));
                // std::cout << "func block added" << std::endl;
                func->add_arg_names(arg_names);
                block->add_statement(func);
                block->add_function(func);
            }
        }
        else if (token.type != Type::statement) {
            // We expect semicolon after bare expression
            // expr;
            block->add_statement(get_next_expr(i));
            if (tokens[i++].type != Type::semi_colon)
                throw UnexpectedToken(tokens[i-1]);
        }
    }
    return block;
}