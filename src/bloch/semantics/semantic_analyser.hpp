#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../ast/ast.hpp"
#include "../error/bloch_runtime_error.hpp"
#include "type_system.hpp"

namespace bloch {

    class SemanticAnalyser : public ASTVisitor {
       public:
        void analyse(Program& program);

        void visit(ImportStatement&) override;
        void visit(VariableDeclaration& node) override;
        void visit(BlockStatement& node) override;
        void visit(ExpressionStatement& node) override;
        void visit(ReturnStatement& node) override;
        void visit(IfStatement& node) override;
        void visit(ForStatement& node) override;
        void visit(WhileStatement& node) override;
        void visit(EchoStatement& node) override;
        void visit(ResetStatement& node) override;
        void visit(MeasureStatement& node) override;
        void visit(AssignmentStatement& node) override;

        void visit(BinaryExpression& node) override;
        void visit(UnaryExpression& node) override;
        void visit(LiteralExpression& node) override;
        void visit(VariableExpression& node) override;
        void visit(CallExpression& node) override;
        void visit(IndexExpression& node) override;
        void visit(ParenthesizedExpression& node) override;
        void visit(MeasureExpression& node) override;
        void visit(AssignmentExpression& node) override;

        void visit(PrimitiveType& node) override;
        void visit(ArrayType& node) override;
        void visit(VoidType& node) override;

        void visit(Parameter& node) override;
        void visit(AnnotationNode& node) override;
        void visit(FunctionDeclaration& node) override;
        void visit(Program& node) override;

       private:
        SymbolTable m_symbols;
        ValueType m_currentReturnType = ValueType::Unknown;
        std::unordered_set<std::string> m_functions;

        struct FunctionInfo {
            ValueType returnType = ValueType::Unknown;
            std::vector<ValueType> paramTypes;
        };
        std::unordered_map<std::string, FunctionInfo> m_functionInfo;

        void beginScope();
        void endScope();
        void declare(const std::string& name, bool isFinal, ValueType type);
        bool isDeclared(const std::string& name) const;
        void declareFunction(const std::string& name);
        bool isFunctionDeclared(const std::string& name) const;
        bool isFinal(const std::string& name) const;
        size_t getFunctionParamCount(const std::string& name) const;
        std::vector<ValueType> getFunctionParamTypes(const std::string& name) const;
        ValueType getVariableType(const std::string& name) const;
        bool returnsVoid(const std::string& name) const;
    };

}