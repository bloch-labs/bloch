// Copyright 2025 Akshay Pal (https://bloch-labs.com)
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
