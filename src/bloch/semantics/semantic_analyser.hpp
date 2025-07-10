#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../ast/ast.hpp"
#include "../error/bloch_runtime_error.hpp"

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
        void visit(ConstructorCallExpression& node) override;
        void visit(MemberAccessExpression& node) override;

        void visit(PrimitiveType& node) override;
        void visit(LogicalType& node) override;
        void visit(ArrayType& node) override;
        void visit(VoidType& node) override;
        void visit(ObjectType& node) override;

        void visit(Parameter& node) override;
        void visit(AnnotationNode& node) override;
        void visit(FunctionDeclaration& node) override;
        void visit(ClassDeclaration& node) override;
        void visit(Program& node) override;

       private:
        struct VariableInfo {
            bool isFinal = false;
        };

        std::vector<std::unordered_map<std::string, VariableInfo>> m_scopes;
        Type* m_currentReturnType = nullptr;
        std::unordered_set<std::string> m_functions;

        void beginScope();
        void endScope();
        void declare(const std::string& name, bool isFinal);
        bool isDeclared(const std::string& name) const;
        void declareFunction(const std::string& name);
        bool isFunctionDeclared(const std::string& name) const;
        bool isFinal(const std::string& name) const;
    };

}