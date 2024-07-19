#include <unordered_map>
#include <string>
#include <iostream>
#include <sstream>

#include "ASTNode.hpp"
#include "Environment.hpp"
#include "Exception.hpp"
#include "ValueSum.hpp"

// Environment::Environment() : parent(), env() { 
//     parent = nullptr; 
// }

Environment::Environment() : env() {}

Environment::~Environment() {
    env.clear();
}

// void Environment::set_parent(std::shared_ptr<Environment> p) {
//     parent = p;
// }

void Environment::add(const std::string &symbol, ValueSum value) { env[symbol] = value; }

ValueSum Environment::get(const std::string &symbol){
    if (env.find(symbol) == env.end()) {
        // if (parent) parent->get(symbol);
        throw RuntimeError("Runtime error: unknown identifier " + symbol);
    }
    return env[symbol];
}

void Environment::copy(std::shared_ptr<Environment> other) {
    for (auto [k, v]: other->env) {  
        add(k, v);
    }    
}

void Environment::clear() {
    env.clear();
}

std::string Environment::to_string() {
    std::ostringstream oss;
    for (auto [k, v]: env) {  
        oss << k << ": " << vsum_to_string(v) << "\n";
    }
    return oss.str();
}

bool Environment::contains(const std::string &symbol) {
    return env.find(symbol) != env.end();
}

