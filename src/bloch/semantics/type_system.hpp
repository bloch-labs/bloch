#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace bloch {

    enum class ValueType { Int, Float, String, Char, Qubit, Bit, Void, Unknown };

    ValueType typeFromString(const std::string& name);
    std::string typeToString(ValueType type);

    struct SymbolInfo {
        bool isFinal = false;
        ValueType type = ValueType::Unknown;
    };

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