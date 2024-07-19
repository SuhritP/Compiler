#include <iostream>
#include <deque>
#include <sstream>
#include <string>
#include <cctype>
#include <iomanip>
#include <algorithm>
#include "./lib/Lexer.hpp"
#include "./lib/Exception.hpp"

/*
 * This program tokenizes the input from the standard input stream using the Lexer class.
 * The tokens are then displayed in the format: [line number][column number][token text].
 * If an ScryptException occurs during tokenization, the program outputs the error message 
 * and exits with the error code obtained from the exception.
 */
int main()
{
    Lexer lexer;
    std::deque<Token> tokens;

    try {
        tokens = lexer.tokenize(std::cin);
    }
    catch(ScryptException& e) {
        std::cout << e.what() << std::endl;
        return e.code();  // Exit with the error code from the exception
    }

    for (Token token : tokens) {
        std::cout << std::right << std::setw(4) << token.line_number 
                  << std::setw(5) << token.column_number 
                  << "  " << token.text << std::endl;
    }

    return 0;
}
