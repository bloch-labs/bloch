#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include "type_system.hpp"

namespace bloch {
    struct BuiltInGate {
        std::string name;
        std::vector<ValueType> paramTypes;
        ValueType returnType;
    };

    extern const std::unordered_map<std::string, BuiltInGate> builtInGates;
}