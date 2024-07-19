#pragma once

#include <stdexcept>
#include <string>

#include "ASTNode.hpp"
#include "Lexer.hpp"
#include "ValueSum.hpp"

// Base Exception class for Scrypt Project
class ScryptException : public std::exception {
    public: 
        const char * what();
        virtual size_t code() = 0;
    protected:
        std::string message;
};

class SyntaxError : public ScryptException {
    public: 
        SyntaxError(size_t line, size_t col);

        size_t code();
};

class UnexpectedToken : public ScryptException {
    public:
        UnexpectedToken(Token t, size_t offset = 0);

        size_t code();
};

class RuntimeError : public ScryptException {
    public:
        RuntimeError(std::string error_message);

        size_t code();
};

class ReturnThrow : public std::exception {
    public:
        ReturnThrow(ValueSum ret);
        
        ValueSum get_ret();
    protected:
        ValueSum ret;
};