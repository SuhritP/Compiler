#include <iostream>
#include <deque>
#include <sstream>
#include <string>
#include <cctype>
#include <iomanip>
#include <algorithm>

#include "./lib/Lexer.hpp"
#include "./lib/Exception.hpp"
#include "./lib/Infix.hpp"
#include "./lib/Scrypt.hpp"


int main() {
    Lexer lexer;
    std::deque<Token> tokens;

    try {
        tokens = lexer.tokenize(std::cin);

        // for (Token token : tokens) {
        //     std::cout << std::right << std::setw(4) << token.line_number 
        //             << std::setw(5) << token.column_number 
        //             << "  " << token.text << std::endl;
        // }

        Scrypt::Parser parser(tokens);
        auto x = parser.parse();
        std::cout << x->to_string() << std::endl;
    }
    catch(ScryptException& e) {
        std::cout << e.what() << std::endl;
        return e.code();  // Exit with the error code from the exception
    }

    return 0;
}
