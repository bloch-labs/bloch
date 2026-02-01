// Copyright 2025-2026 Akshay Pal (https://bloch-labs.com)
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

#include "bloch/compiler/semantics/type_system.hpp"

namespace bloch::compiler {

    ValueType typeFromString(const std::string& name) {
        // Map source-level type names to our compact enum.
        if (name == "int")
            return ValueType::Int;
        if (name == "long")
            return ValueType::Long;
        if (name == "float")
            return ValueType::Float;
        if (name == "string")
            return ValueType::String;
        if (name == "char")
            return ValueType::Char;
        if (name == "qubit")
            return ValueType::Qubit;
        if (name == "bit")
            return ValueType::Bit;
        if (name == "boolean")
            return ValueType::Boolean;
        if (name == "null")
            return ValueType::Null;
        if (name == "void")
            return ValueType::Void;
        return ValueType::Unknown;
    }

    std::string typeToString(ValueType type) {
        // The reverse mapping comes in handy for diagnostics.
        switch (type) {
            case ValueType::Int:
                return "int";
            case ValueType::Long:
                return "long";
            case ValueType::Float:
                return "float";
            case ValueType::String:
                return "string";
            case ValueType::Char:
                return "char";
            case ValueType::Qubit:
                return "qubit";
            case ValueType::Bit:
                return "bit";
            case ValueType::Boolean:
                return "boolean";
            case ValueType::Null:
                return "null";
            case ValueType::Void:
                return "void";
            default:
                return "unknown";
        }
    }

    void SymbolTable::beginScope() { m_scopes.emplace_back(); }

    void SymbolTable::endScope() {
        if (!m_scopes.empty())
            m_scopes.pop_back();
    }

    void SymbolTable::declare(const std::string& name, bool isFinal, ValueType type,
                              const std::string& className, bool isTypeName) {
        if (m_scopes.empty())
            return;
        m_scopes.back()[name] = SymbolInfo{isFinal, type, std::nullopt, className, isTypeName};
    }

    bool SymbolTable::isDeclared(const std::string& name) const {
        for (auto it = m_scopes.rbegin(); it != m_scopes.rend(); ++it) {
            if (it->count(name))
                return true;
        }
        return false;
    }

    bool SymbolTable::isFinal(const std::string& name) const {
        for (auto it = m_scopes.rbegin(); it != m_scopes.rend(); ++it) {
            auto found = it->find(name);
            if (found != it->end())
                return found->second.isFinal;
        }
        return false;
    }

    ValueType SymbolTable::getType(const std::string& name) const {
        for (auto it = m_scopes.rbegin(); it != m_scopes.rend(); ++it) {
            auto found = it->find(name);
            if (found != it->end())
                return found->second.type;
        }
        return ValueType::Unknown;
    }

    std::string SymbolTable::getClassName(const std::string& name) const {
        for (auto it = m_scopes.rbegin(); it != m_scopes.rend(); ++it) {
            auto found = it->find(name);
            if (found != it->end())
                return found->second.className;
        }
        return "";
    }

    bool SymbolTable::isTypeName(const std::string& name) const {
        for (auto it = m_scopes.rbegin(); it != m_scopes.rend(); ++it) {
            auto found = it->find(name);
            if (found != it->end())
                return found->second.isTypeName;
        }
        return false;
    }

    std::optional<int> SymbolTable::getConstInt(const std::string& name) const {
        for (auto it = m_scopes.rbegin(); it != m_scopes.rend(); ++it) {
            auto found = it->find(name);
            if (found != it->end())
                return found->second.constInt;
        }
        return std::nullopt;
    }

    void SymbolTable::setConstInt(const std::string& name, int value) {
        for (auto it = m_scopes.rbegin(); it != m_scopes.rend(); ++it) {
            auto found = it->find(name);
            if (found != it->end()) {
                found->second.constInt = value;
                return;
            }
        }
    }

}  // namespace bloch::compiler
