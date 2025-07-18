#pragma once

#include <string>
#include <unordered_map>
#include "../ast/ast.hpp"
#include "../semantics/built_ins.hpp"

namespace bloch {

    class CppGenerator {
       public:
        explicit CppGenerator(const std::unordered_map<const Expression*, int>& m) : m_measure(m) {}
        std::string generate(Program& program);

       private:
        const std::unordered_map<const Expression*, int>& m_measure;
        int m_indent = 0;
        std::string m_code;
        void indent();
        std::string cppType(Type* t);
        void genFunction(FunctionDeclaration* fn);
        std::string genExpr(Expression* e);
        void genStmt(Statement* s);
        bool containsQubit(Type* t);
        bool shouldSkipFunction(FunctionDeclaration* fn);
    };

}