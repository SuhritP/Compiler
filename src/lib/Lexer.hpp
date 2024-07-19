#pragma once

#include <iostream>
#include <string>
#include <deque>

enum class Type {
    left_paren,    
    right_paren,
    left_curly,
    right_curly,
    comma,
    semi_colon,
    boolean,
    statement,
    null,
    number,        
    op,    
    identifier,
    assignment,        
    END            
};

struct Token {
    std::string text;
    int line_number;
    int column_number;
    Type type;
};

// The Lexer class is responsible for converting an input stream into individual tokens.
// It uses the Token structure to represent each token and a deque container to store the sequence of tokens.
class Lexer {
    public:
        Lexer() = default;

        // This function tokenizes the provided input stream and returns a deque of Tokens.
        // The tokenizing process involves identifying different types of characters and sequences 
        // and classifying them into categories like parentheses, numbers, operators, etc.
        std::deque<Token> tokenize(std::istream &stream, int offset = 0);
};
