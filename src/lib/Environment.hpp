#pragma once

#include <string>
#include <unordered_map>

#include "ASTNode.hpp"
#include "ValueSum.hpp"


class Environment;

class Environment{
    public:
        Environment();
        ~Environment();
        // void set_parent(std::shared_ptr<Environment> p);
        void add(const std::string &symbol, ValueSum value);
        void copy(std::shared_ptr<Environment> other);
        void clear();
        std::string to_string();
        ValueSum get(const std::string &symbol);
        bool contains(const std::string &symbol);
    // protected:
        // std::shared_ptr<Environment> parent;
        std::unordered_map<std::string, ValueSum> env;
};