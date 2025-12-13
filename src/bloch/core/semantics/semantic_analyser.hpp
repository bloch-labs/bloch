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
#include <unordered_set>
#include <vector>

#include "bloch/core/ast/ast.hpp"
#include "bloch/core/semantics/type_system.hpp"
#include "bloch/support/error/bloch_error.hpp"

namespace bloch::core {

    // The semantic pass checks name usage, types at a coarse level,
    // function contracts, and a few language rules (like @tracked constraints).
    // It also records function signatures for later runtime validation.
    class SemanticAnalyser : public ASTVisitor {
       public:
        void analyse(Program& program);

        // Visitors
        void visit(VariableDeclaration& node) override;
        void visit(BlockStatement& node) override;
        void visit(ExpressionStatement& node) override;
        void visit(ReturnStatement& node) override;
        void visit(IfStatement& node) override;
        void visit(TernaryStatement& node) override;
        void visit(ForStatement& node) override;
        void visit(WhileStatement& node) override;
        void visit(EchoStatement& node) override;
        void visit(ResetStatement& node) override;
        void visit(MeasureStatement& node) override;
        void visit(AssignmentStatement& node) override;

        void visit(BinaryExpression& node) override;
        void visit(UnaryExpression& node) override;
        void visit(PostfixExpression& node) override;
        void visit(LiteralExpression& node) override;
        void visit(VariableExpression& node) override;
        void visit(CallExpression& node) override;
        void visit(IndexExpression& node) override;
        void visit(ArrayLiteralExpression& node) override;
        void visit(ParenthesizedExpression& node) override;
        void visit(MeasureExpression& node) override;
        void visit(AssignmentExpression& node) override;
        void visit(ArrayAssignmentExpression& node) override;

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
        bool m_foundReturn = false;
        std::unordered_set<std::string> m_functions;

        struct FunctionInfo {
            ValueType returnType = ValueType::Unknown;
            std::vector<ValueType> paramTypes;
        };
        std::unordered_map<std::string, FunctionInfo> m_functionInfo;

        // Helpers
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

        // Type inference
        ValueType inferType(Expression* expr) const;
        std::optional<int> evaluateConstInt(Expression* expr) const;
    };

}  // namespace bloch::core
