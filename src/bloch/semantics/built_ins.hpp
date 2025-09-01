#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include "type_system.hpp"

namespace bloch {
    // Built-in gate signatures used by the semantic pass and runtime dispatch.
    struct BuiltInGate {
        std::string name;
        std::vector<ValueType> paramTypes;
        ValueType returnType;
    };

    extern const std::unordered_map<std::string, BuiltInGate> builtInGates;
}
