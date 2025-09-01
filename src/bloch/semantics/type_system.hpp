#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace bloch {

    // A compact type universe for semantic checks and runtime hints.
    enum class ValueType { Int, Float, String, Char, Qubit, Bit, Void, Unknown };

    ValueType typeFromString(const std::string& name);
    std::string typeToString(ValueType type);

    // Per-name metadata tracked across nested scopes.
    struct SymbolInfo {
        bool isFinal = false;
        ValueType type = ValueType::Unknown;
    };

    // Nested-scope symbol table (stack of hash maps).
    class SymbolTable {
       public:
        void beginScope();
        void endScope();
        void declare(const std::string& name, bool isFinal, ValueType type);
        bool isDeclared(const std::string& name) const;
        bool isFinal(const std::string& name) const;
        ValueType getType(const std::string& name) const;

       private:
        std::vector<std::unordered_map<std::string, SymbolInfo>> m_scopes;
    };
}
