#include <string>

#include "Exception.hpp"
#include "Lexer.hpp"
#include "ASTNode.hpp"
#include "ValueSum.hpp"

const char * ScryptException::what() { return message.c_str(); }

SyntaxError::SyntaxError(size_t line, size_t col) {
    message = "Syntax error on line " + std::to_string(line) + " column " + std::to_string(col) + ".";
}
size_t SyntaxError::code() { return 1; }

UnexpectedToken::UnexpectedToken(Token t, size_t offset) {
    message = "Unexpected token at line " + std::to_string(t.line_number) + " column " + std::to_string(t.column_number - offset) + ": " + t.text;
}
size_t UnexpectedToken::code() { return 2; }

RuntimeError::RuntimeError(std::string error_message) {
    message = error_message;
}
size_t RuntimeError::code() { return 3; }

ReturnThrow::ReturnThrow(ValueSum ret) {
    this->ret = ret;
}
ValueSum ReturnThrow::get_ret() { return ret; }