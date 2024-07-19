#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <numeric>
#include <iomanip>
#include <sstream>
#include <cmath>

#include "./lib/Infix.hpp"
#include "./lib/Lexer.hpp"
#include "./lib/Exception.hpp"

int main() {
    std::deque<Token> tokens;
    Lexer lexer;
    
    std::string line;
    std::stringstream ss;
    auto global_sp = std::make_shared<Environment>(Environment());
    while (std::getline(std::cin, line)) {
        // this offsets col number by 1
        ss << "(" << line << ")";
        try {
            tokens = lexer.tokenize(ss, 1);
            // Calc does not support statements
            for (Token token : tokens) {
                if (token.type == Type::statement)
                    throw UnexpectedToken(token);
            }
            Infix::Parser parser(tokens);
            auto x = parser.eval(global_sp);
            if (parser.parse(global_sp)->token.type != Type::op || tokens.front().type != Type::left_paren)
                std::cout << parser.parse(global_sp)->to_string() << std::endl;
            else
                std::cout << parser.parse(global_sp)->to_string() << std::endl;
            std::cout << x.second << std::endl;
        }
        catch(ScryptException& e) {
            std::cout << e.what() << std::endl;
        }
        ss.str(std::string());
        ss.clear();
    }
    return 0;
}
