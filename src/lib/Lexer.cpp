#include <iostream>
#include <deque>
#include <sstream>
#include <string>
#include <cctype>
#include <iomanip>
#include <algorithm>

#include "Lexer.hpp"
#include "Exception.hpp"

std::deque<Token> Lexer::tokenize(std::istream &stream, int offset) {
    // Characters considered one at a time from stream
    std::deque<Token> tokens;

    int line = 1, col = 1 - offset;
    std::string num;
    
    char c;

    // Lambda function to finalize the accumulated number token and reset the accumulator
    auto flush_num = [&]() {
        if (num.empty()) return;
        // Checking for syntax error where a number ends with a '.'
        if (num.back() == '.') 
            throw SyntaxError(line, col + num.length());
        
        // Add the accumulated number as a new token to the tokens deque
        tokens.push_back(Token{num, line, col, Type::number});
        
        // Update column index
        col += num.length();

        // Reset the accumulator
        num.clear();
    };

    // Loop until end of the stream
    while (!stream.eof()) {
        // Read the next character from the stream without skipping white spaces
        stream >> std::noskipws >> c;
        if (stream.eof()) break;

        // Create token based on the character read
        if (c == '(') {
            flush_num();  // Flush any accumulated number
            tokens.push_back(Token{"(", line, col, Type::left_paren});
            col++;
        }
        else if (c == ')') {
            flush_num();
            tokens.push_back(Token{")", line, col, Type::right_paren});
            col++;
        }
        else if (c == '{') {
            flush_num();
            tokens.push_back(Token{"{", line, col, Type::left_curly});
            col++;  
        }
        else if (c == '}') {
            flush_num();
            tokens.push_back(Token{"}", line, col, Type::right_curly});
            col++;  
        }
        else if (c == '\n') {
            flush_num();
            line++;  // Move to next line
            col = 1;  // Reset column index
        }
        else if (isspace(c)) {
            flush_num();
            col++;  // Move to next column
        }
        else if ('0' <= c && c <= '9') {
            // Append the digit to the number accumulator
            num += c;
        }
        else if (c == '.') {
            // Check for errors like '.' without leading digits or multiple '.' in a number
            if (num.empty())
                throw SyntaxError(line, col);
            else if (std::count(num.begin(), num.end(), '.') == 1)
                throw SyntaxError(line, col + num.length());
            else
                num += c;  // Append '.' to the number accumulator
        }
        else if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '&' || c == '^' || c == '|') {
            flush_num();  // Flush any accumulated number
            tokens.push_back(Token{std::string(1, c), line, col, Type::op});  // Add operator as a new token
            col++;
        }
        else if (c == '<' || c == '>') {
            flush_num();
            std::string t = std::string(1, c);
            if (stream.peek() == '=') {
                stream >> c;
                t += c;
            }
            tokens.push_back(Token{t, line, col, Type::op});
            col += t.length();
        }
        else if (isalpha(c) || c == '_') {
            std::string identifier;
            identifier += c;
            while (stream.peek() == '_' || isalnum(stream.peek())) {
                stream >> c;
                identifier += c;
            }
            // Reserved words
            if (identifier == "true" || identifier == "false")
                tokens.push_back(Token{identifier, line, col, Type::boolean});
            else if (identifier == "null")
                tokens.push_back(Token{identifier, line, col, Type::null});
            else if (identifier == "while" || identifier == "print" || identifier == "if" || identifier == "else" || identifier == "return" || identifier == "def")
                tokens.push_back(Token{identifier, line, col, Type::statement});
            else
                tokens.push_back(Token{identifier, line, col, Type::identifier});
            col += identifier.length();
        }
        else if (c == '=') {
            flush_num();
            std::string t = std::string(1, c);
            if (stream.peek() == '=') {
                stream >> c;
                t += c;
                tokens.push_back(Token{t, line, col, Type::op});
            }
            else
                tokens.push_back(Token{"=", line, col, Type::assignment});
            col += t.length();
        }
        else if (c == '!' && stream.peek() == '=') {
            flush_num();
            std::string t = std::string(1, c);
            stream >> c;
            t += c;
            tokens.push_back(Token{t, line, col, Type::op});
            col += t.length();
        }
        else if (c == ',') {
            flush_num();
            tokens.push_back(Token{",", line, col, Type::comma});
            col++;
        }
        else if (c == ';') {
            flush_num();
            tokens.push_back(Token{";", line, col, Type::semi_colon});
            col++;
        }
        else {
            // Any other character is treated as an unexpected token, hence a syntax error
            throw SyntaxError(line, col);
        }
    }
    flush_num();  // Flush any remaining accumulated number

    // Add an "END" token to signify the end of the tokens
    tokens.push_back(Token{"END", line, col, Type::END});

    // Return the generated tokens
    return tokens;
}