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

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace bloch::core {

    // The AST is a compact tree built by the parser. Each node carries
    // its source position and supports a classic visitor for later stages
    // (semantic analysis and execution).
    // Only the essentials live here to keep traversal cheap and predictable.

    // Base Node Interfaces
    class ASTVisitor;

    struct ASTNode {
        int line = 0;
        int column = 0;
        virtual ~ASTNode() = default;
        virtual void accept(ASTVisitor& visitor) = 0;
    };

    struct Statement : public ASTNode {};
    struct Expression : public ASTNode {};
    struct Type : public ASTNode {};

    // Pre-declared Nodes
    struct BlockStatement;
    struct AnnotationNode;
    struct Parameter;

    enum class Visibility { Public, Private, Protected };

    // Variable Declaration
    struct VariableDeclaration : public Statement {
        std::string name;
        std::unique_ptr<Type> varType;
        std::unique_ptr<Expression> initializer;
        std::vector<std::unique_ptr<AnnotationNode>> annotations;
        bool isFinal = false;
        bool isTracked = false;

        VariableDeclaration() = default;
        void accept(ASTVisitor& visitor) override;
    };

    // Block Statement
    // {...}
    struct BlockStatement : public Statement {
        std::vector<std::unique_ptr<Statement>> statements;

        BlockStatement() = default;
        void accept(ASTVisitor& visitor) override;
    };

    // Expression Statement
    struct ExpressionStatement : public Statement {
        std::unique_ptr<Expression> expression;

        ExpressionStatement() = default;
        void accept(ASTVisitor& visitor) override;
    };

    // Return Statement
    struct ReturnStatement : public Statement {
        std::unique_ptr<Expression> value;

        ReturnStatement() = default;
        void accept(ASTVisitor& visitor) override;
    };

    // If Statement
    struct IfStatement : public Statement {
        std::unique_ptr<Expression> condition;
        std::unique_ptr<Statement> thenBranch;
        std::unique_ptr<Statement> elseBranch;

        IfStatement() = default;
        void accept(ASTVisitor& visitor) override;
    };

    // For Statement
    struct ForStatement : public Statement {
        std::unique_ptr<Statement> initializer;
        std::unique_ptr<Expression> condition;
        std::unique_ptr<Expression> increment;
        std::unique_ptr<Statement> body;

        ForStatement() = default;
        void accept(ASTVisitor& visitor) override;
    };

    // While Statement
    struct WhileStatement : public Statement {
        std::unique_ptr<Expression> condition;
        std::unique_ptr<Statement> body;

        WhileStatement() = default;
        void accept(ASTVisitor& visitor) override;
    };

    // Echo Statement
    // Print a value at runtime; useful for examples and quick debugging.
    struct EchoStatement : public Statement {
        std::unique_ptr<Expression> value;

        EchoStatement() = default;
        void accept(ASTVisitor& visitor) override;
    };

    // Reset Statement
    struct ResetStatement : public Statement {
        std::unique_ptr<Expression> target;

        ResetStatement() = default;
        void accept(ASTVisitor& visitor) override;
    };

    // Measure Statement
    // Measure a qubit and push the result into classical memory.
    struct MeasureStatement : public Statement {
        std::unique_ptr<Expression> qubit;

        MeasureStatement() = default;
        void accept(ASTVisitor& visitor) override;
    };

    // destroy expr;
    struct DestroyStatement : public Statement {
        std::unique_ptr<Expression> target;

        DestroyStatement() = default;
        void accept(ASTVisitor& visitor) override;
    };

    // Ternary Statement
    // cond ? thenBranch : elseBranch
    struct TernaryStatement : public Statement {
        std::unique_ptr<Expression> condition;
        std::unique_ptr<Statement> thenBranch;
        std::unique_ptr<Statement> elseBranch;

        TernaryStatement() = default;
        void accept(ASTVisitor& visitor) override;
    };

    // Assignment
    struct AssignmentStatement : public Statement {
        std::string name;
        std::unique_ptr<Expression> value;

        AssignmentStatement() = default;
        void accept(ASTVisitor& visitor) override;
    };

    // Binary Expression
    struct BinaryExpression : public Expression {
        std::string op;
        std::unique_ptr<Expression> left;
        std::unique_ptr<Expression> right;

        BinaryExpression(const std::string& op, std::unique_ptr<Expression> left,
                         std::unique_ptr<Expression> right)
            : op(op), left(std::move(left)), right(std::move(right)) {}
        void accept(ASTVisitor& visitor) override;
    };

    // Unary Expression
    struct UnaryExpression : public Expression {
        std::string op;
        std::unique_ptr<Expression> right;

        UnaryExpression(const std::string& op, std::unique_ptr<Expression> right)
            : op(op), right(std::move(right)) {}
        void accept(ASTVisitor& visitor) override;
    };

    // Postfix Expression
    // Support for i++ and i--
    struct PostfixExpression : public Expression {
        std::string op;
        std::unique_ptr<Expression> left;

        PostfixExpression(const std::string& op, std::unique_ptr<Expression> left)
            : op(op), left(std::move(left)) {}
        void accept(ASTVisitor& visitor) override;
    };

    // Literal Expression
    struct LiteralExpression : public Expression {
        std::string value;
        std::string literalType;

        LiteralExpression(const std::string& value, const std::string& type)
            : value(value), literalType(type) {}
        void accept(ASTVisitor& visitor) override;
    };

    // Variable Expression
    struct VariableExpression : public Expression {
        std::string name;

        VariableExpression(const std::string& name) : name(name) {}
        void accept(ASTVisitor& visitor) override;
    };

    // Call Expression
    struct CallExpression : public Expression {
        std::unique_ptr<Expression> callee;
        std::vector<std::unique_ptr<Expression>> arguments;

        CallExpression(std::unique_ptr<Expression> callee,
                       std::vector<std::unique_ptr<Expression>> args)
            : callee(std::move(callee)), arguments(std::move(args)) {}
        void accept(ASTVisitor& visitor) override;
    };

    // Member access expression: object.member
    struct MemberAccessExpression : public Expression {
        std::unique_ptr<Expression> object;
        std::string member;

        MemberAccessExpression() = default;
        void accept(ASTVisitor& visitor) override;
    };

    // Object creation
    struct NewExpression : public Expression {
        std::unique_ptr<Type> classType;
        std::vector<std::unique_ptr<Expression>> arguments;

        NewExpression() = default;
        void accept(ASTVisitor& visitor) override;
    };

    // this
    struct ThisExpression : public Expression {
        ThisExpression() = default;
        void accept(ASTVisitor& visitor) override;
    };

    // super
    struct SuperExpression : public Expression {
        SuperExpression() = default;
        void accept(ASTVisitor& visitor) override;
    };

    // Index Expression a[i]
    // Indexing into arrays (bounds/typing checked later).
    struct IndexExpression : public Expression {
        std::unique_ptr<Expression> collection;
        std::unique_ptr<Expression> index;

        IndexExpression() = default;
        void accept(ASTVisitor& visitor) override;
    };

    // Array Literal Expression
    // {a, b, c} literal; type is inferred or validated during semantics/runtime.
    struct ArrayLiteralExpression : public Expression {
        std::vector<std::unique_ptr<Expression>> elements;

        ArrayLiteralExpression(std::vector<std::unique_ptr<Expression>> elems)
            : elements(std::move(elems)) {}
        void accept(ASTVisitor& visitor) override;
    };

    // Parenthesized Expression
    // (expr) keeps source intent; may be useful for diagnostics.
    struct ParenthesizedExpression : public Expression {
        std::unique_ptr<Expression> expression;

        ParenthesizedExpression(std::unique_ptr<Expression> expr) : expression(std::move(expr)) {}
        void accept(ASTVisitor& visitor) override;
    };

    // Measure Expression
    // Inline measurement returns a bit value.
    struct MeasureExpression : public Expression {
        std::unique_ptr<Expression> qubit;

        MeasureExpression(std::unique_ptr<Expression> qubit) : qubit(std::move(qubit)) {}
        void accept(ASTVisitor& visitor) override;
    };

    // Assignment Expression
    struct AssignmentExpression : public Expression {
        std::string name;
        std::unique_ptr<Expression> value;

        AssignmentExpression(std::string name, std::unique_ptr<Expression> value)
            : name(std::move(name)), value(std::move(value)) {}
        void accept(ASTVisitor& visitor) override;
    };

    // object.member = value
    struct MemberAssignmentExpression : public Expression {
        std::unique_ptr<Expression> object;
        std::string member;
        std::unique_ptr<Expression> value;

        MemberAssignmentExpression(std::unique_ptr<Expression> object, std::string member,
                                   std::unique_ptr<Expression> value)
            : object(std::move(object)), member(std::move(member)), value(std::move(value)) {}
        void accept(ASTVisitor& visitor) override;
    };

    // Array element assignment expression: collection[index] = value
    struct ArrayAssignmentExpression : public Expression {
        std::unique_ptr<Expression> collection;
        std::unique_ptr<Expression> index;
        std::unique_ptr<Expression> value;

        ArrayAssignmentExpression(std::unique_ptr<Expression> collection,
                                  std::unique_ptr<Expression> index,
                                  std::unique_ptr<Expression> value)
            : collection(std::move(collection)), index(std::move(index)), value(std::move(value)) {}
        void accept(ASTVisitor& visitor) override;
    };

    // Type Nodes
    struct PrimitiveType : public Type {
        std::string name;

        PrimitiveType(const std::string& name) : name(name) {}
        void accept(ASTVisitor& visitor) override;
    };

    // User-defined or qualified type
    struct NamedType : public Type {
        std::vector<std::string> nameParts;

        explicit NamedType(std::vector<std::string> parts) : nameParts(std::move(parts)) {}
        void accept(ASTVisitor& visitor) override;
    };

    // Array type
    struct ArrayType : public Type {
        std::unique_ptr<Type> elementType;
        // Optional fixed size for the array. If negative, size is unspecified.
        int size = -1;
        // Optional raw size expression for cases where the size is derived from a
        // compile-time constant (e.g. final int n = 4; int[n] a;). Semantic
        // analysis resolves this to 'size'.
        std::unique_ptr<Expression> sizeExpression;

        ArrayType(std::unique_ptr<Type> elementType, int size = -1,
                  std::unique_ptr<Expression> sizeExpr = nullptr)
            : elementType(std::move(elementType)),
              size(size),
              sizeExpression(std::move(sizeExpr)) {}
        void accept(ASTVisitor& visitor) override;
    };

    // void
    struct VoidType : public Type {
        VoidType() = default;
        void accept(ASTVisitor& visitor) override;
    };

    // Parameter
    struct Parameter : public ASTNode {
        std::string name;
        std::unique_ptr<Type> type;

        Parameter() = default;
        void accept(ASTVisitor& visitor) override;
    };

    // Annotation
    struct AnnotationNode : public ASTNode {
        std::string name;
        std::string value = std::string{""};
        bool isFunctionAnnotation = false;
        bool isVariableAnnotation = false;

        AnnotationNode() = default;
        AnnotationNode(std::string& name, std::string& value) : name(name), value(value) {}
        void accept(ASTVisitor& visitor) override;
    };

    // Import declaration
    struct ImportDeclaration : public ASTNode {
        std::vector<std::string> path;

        ImportDeclaration() = default;
        void accept(ASTVisitor& visitor) override;
    };

    // Class Members
    struct ClassMember : public ASTNode {
        Visibility visibility = Visibility::Public;
    };

    // Fields
    struct FieldDeclaration : public ClassMember {
        std::string name;
        std::unique_ptr<Type> fieldType;
        std::unique_ptr<Expression> initializer;
        std::vector<std::unique_ptr<AnnotationNode>> annotations;
        bool isFinal = false;
        bool isStatic = false;
        bool isTracked = false;

        FieldDeclaration() = default;
        void accept(ASTVisitor& visitor) override;
    };

    // Methods
    struct MethodDeclaration : public ClassMember {
        std::string name;
        std::vector<std::unique_ptr<Parameter>> params;
        std::unique_ptr<Type> returnType;
        std::unique_ptr<BlockStatement> body;
        std::vector<std::unique_ptr<AnnotationNode>> annotations;
        bool hasQuantumAnnotation = false;
        bool isStatic = false;
        bool isVirtual = false;
        bool isOverride = false;

        MethodDeclaration() = default;
        void accept(ASTVisitor& visitor) override;
    };

    // Constructor
    struct ConstructorDeclaration : public ClassMember {
        std::vector<std::unique_ptr<Parameter>> params;
        std::unique_ptr<BlockStatement> body;
        bool isDefault = false;

        ConstructorDeclaration() = default;
        void accept(ASTVisitor& visitor) override;
    };

    // Destructor
    struct DestructorDeclaration : public ClassMember {
        std::unique_ptr<BlockStatement> body;
        bool isDefault = false;

        DestructorDeclaration() = default;
        void accept(ASTVisitor& visitor) override;
    };

    // Class Declaration
    struct ClassDeclaration : public ASTNode {
        std::string name;
        std::vector<std::string> baseName;
        bool isStatic = false;
        bool isAbstract = false;
        std::vector<std::unique_ptr<ClassMember>> members;

        ClassDeclaration() = default;
        void accept(ASTVisitor& visitor) override;
    };

    // Function Declaration
    struct FunctionDeclaration : public ASTNode {
        std::string name;
        std::vector<std::unique_ptr<Parameter>> params;
        std::unique_ptr<Type> returnType;
        std::unique_ptr<BlockStatement> body;
        std::vector<std::unique_ptr<AnnotationNode>> annotations;
        bool hasQuantumAnnotation = false;
        bool hasShotsAnnotation = false;

        FunctionDeclaration() = default;
        void accept(ASTVisitor& visitor) override;
    };

    // Program
    struct Program : public ASTNode {
        std::vector<std::unique_ptr<ImportDeclaration>> imports;
        std::vector<std::unique_ptr<ClassDeclaration>> classes;
        std::vector<std::unique_ptr<FunctionDeclaration>> functions;
        std::vector<std::unique_ptr<Statement>> statements;
        std::pair<bool, int> shots;

        Program() = default;

        void accept(ASTVisitor& visitor) override;
    };

    // Visitor Interface
    class ASTVisitor {
       public:
        virtual ~ASTVisitor() = default;

        virtual void visit(VariableDeclaration& node) = 0;
        virtual void visit(BlockStatement& node) = 0;
        virtual void visit(ExpressionStatement& node) = 0;
        virtual void visit(ReturnStatement& node) = 0;
        virtual void visit(IfStatement& node) = 0;
        virtual void visit(ForStatement& node) = 0;
        virtual void visit(WhileStatement& node) = 0;
        virtual void visit(EchoStatement& node) = 0;
        virtual void visit(ResetStatement& node) = 0;
        virtual void visit(MeasureStatement& node) = 0;
        virtual void visit(DestroyStatement& node) = 0;
        virtual void visit(TernaryStatement& node) = 0;
        virtual void visit(AssignmentStatement& node) = 0;

        virtual void visit(BinaryExpression& node) = 0;
        virtual void visit(UnaryExpression& node) = 0;
        virtual void visit(PostfixExpression& node) = 0;
        virtual void visit(LiteralExpression& node) = 0;
        virtual void visit(VariableExpression& node) = 0;
        virtual void visit(CallExpression& node) = 0;
        virtual void visit(MemberAccessExpression& node) = 0;
        virtual void visit(NewExpression& node) = 0;
        virtual void visit(ThisExpression& node) = 0;
        virtual void visit(SuperExpression& node) = 0;
        virtual void visit(IndexExpression& node) = 0;
        virtual void visit(ArrayLiteralExpression& node) = 0;
        virtual void visit(ParenthesizedExpression& node) = 0;
        virtual void visit(MeasureExpression& node) = 0;
        virtual void visit(AssignmentExpression& node) = 0;
        virtual void visit(MemberAssignmentExpression& node) = 0;
        virtual void visit(ArrayAssignmentExpression& node) = 0;

        virtual void visit(PrimitiveType& node) = 0;
        virtual void visit(NamedType& node) = 0;
        virtual void visit(ArrayType& node) = 0;
        virtual void visit(VoidType& node) = 0;

        virtual void visit(Parameter& node) = 0;
        virtual void visit(AnnotationNode& node) = 0;
        virtual void visit(ImportDeclaration& node) = 0;
        virtual void visit(FieldDeclaration& node) = 0;
        virtual void visit(MethodDeclaration& node) = 0;
        virtual void visit(ConstructorDeclaration& node) = 0;
        virtual void visit(DestructorDeclaration& node) = 0;
        virtual void visit(ClassDeclaration& node) = 0;
        virtual void visit(FunctionDeclaration& node) = 0;
        virtual void visit(Program& node) = 0;
    };

    // Inline accept implementations
    inline void VariableDeclaration::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void BlockStatement::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void ExpressionStatement::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void ReturnStatement::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void IfStatement::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void ForStatement::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void WhileStatement::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void EchoStatement::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void ResetStatement::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void MeasureStatement::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void DestroyStatement::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void TernaryStatement::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void AssignmentStatement::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void BinaryExpression::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void UnaryExpression::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void PostfixExpression::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void LiteralExpression::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void VariableExpression::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void CallExpression::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void MemberAccessExpression::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void NewExpression::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void ThisExpression::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void SuperExpression::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void IndexExpression::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void ArrayLiteralExpression::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void ParenthesizedExpression::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void MeasureExpression::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void AssignmentExpression::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void MemberAssignmentExpression::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void ArrayAssignmentExpression::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void PrimitiveType::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void NamedType::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void ArrayType::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void VoidType::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void Parameter::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void AnnotationNode::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void ImportDeclaration::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void FieldDeclaration::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void MethodDeclaration::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void ConstructorDeclaration::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void DestructorDeclaration::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void ClassDeclaration::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void FunctionDeclaration::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void Program::accept(ASTVisitor& visitor) { visitor.visit(*this); }
}  // namespace bloch::core
