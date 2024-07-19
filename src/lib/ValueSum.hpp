#pragma once

#include <variant>
#include <memory>
#include <string>

#include "ASTNode.hpp"

class Function;

struct ValueSum : public std::variant<
    double,
    bool,
    std::nullptr_t,
    std::weak_ptr<Function>> {

};

bool is_bool(ValueSum val);
bool is_number(ValueSum val);
bool is_null(ValueSum val);
bool is_function(ValueSum val);
bool get_bool(ValueSum val);
double get_number(ValueSum val);
std::shared_ptr<Function> get_function(ValueSum val);
std::string vsum_to_string(ValueSum val);