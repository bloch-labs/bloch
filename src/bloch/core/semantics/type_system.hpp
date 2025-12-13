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

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace bloch::core {

    // A compact type universe for semantic checks and runtime hints.
    enum class ValueType { Int, Float, String, Char, Qubit, Bit, Void, Unknown };

    ValueType typeFromString(const std::string& name);
    std::string typeToString(ValueType type);

    // Per-name metadata tracked across nested scopes.
    struct SymbolInfo {
        bool isFinal = false;
        ValueType type = ValueType::Unknown;
        std::optional<int> constInt;
        std::string className;    // Non-empty for user-defined classes
        bool isTypeName = false;  // True when the symbol represents a type (e.g. class name)
    };

    // Nested-scope symbol table (stack of hash maps).
    class SymbolTable {
       public:
        void beginScope();
        void endScope();
        void declare(const std::string& name, bool isFinal, ValueType type,
                     const std::string& className = "", bool isTypeName = false);
        bool isDeclared(const std::string& name) const;
        bool isFinal(const std::string& name) const;
        ValueType getType(const std::string& name) const;
        std::string getClassName(const std::string& name) const;
        bool isTypeName(const std::string& name) const;
        std::optional<int> getConstInt(const std::string& name) const;
        void setConstInt(const std::string& name, int value);

       private:
        std::vector<std::unordered_map<std::string, SymbolInfo>> m_scopes;
    };
}  // namespace bloch::core
