#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace bloch {
    struct BuiltInGate {
        std::string name;
        std::vector<std::string> paramTypes;
        std::string returnType;
    };

    extern const std::unordered_map<std::string, BuiltInGate> builtInGates;
}