#include <variant>
#include <memory>
#include <string>

#include "ASTNode.hpp"
#include "ValueSum.hpp"
#include "Exception.hpp"

bool is_bool(ValueSum val) { return std::holds_alternative<bool>(val); }
bool is_number(ValueSum val) { return std::holds_alternative<double>(val); }
bool is_null(ValueSum val) { return std::holds_alternative<std::nullptr_t>(val); }
bool is_function(ValueSum val) { return std::holds_alternative<std::weak_ptr<Function>>(val); }
bool get_bool(ValueSum val) {
    if (!is_bool(val))
        throw RuntimeError("Runtime error: invalid operand type.");
    return std::get<bool>(val);
}
double get_number(ValueSum val) {
    if (!is_number(val))
        throw RuntimeError("Runtime error: invalid operand type.");
    return std::get<double>(val);
}
std::shared_ptr<Function> get_function(ValueSum val) {
    if (!is_function(val))
        throw RuntimeError("Runtime error: invalid operand type.");
    return std::get<std::weak_ptr<Function>>(val).lock();
}
std::string vsum_to_string(ValueSum val) {
    std::ostringstream stream;
    if (is_number(val))
        stream << get_number(val);
    else if (is_null(val))
        stream << "null";
    else if (is_bool(val))
        stream << (get_bool(val) ? "true" : "false");
    return stream.str();
}