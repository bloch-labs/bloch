#pragma once

#include <memory>
#include <string>
#include <vector>

namespace bloch {

    // Base Node Interfaces
    struct ASTNode {
        virtual ~ASTNode() = default;
    };

    struct Statement : public ASTNode {};
    struct Expression : public ASTNode {};
    struct Type : public ASTNode {};

    // Pre-declared Nodes
    struct BlockStatement;
    struct AnnotationNode;
    struct Parameter;

    // Import Statement
    struct ImportStatement : public Statement {
        std::string module;

        ImportStatement() = default;
    };

    // Variable Declaration
    struct VariableDeclaration : public Statement {
        std::string name;
        std::string access;
        std::unique_ptr<Type> varType;
        std::unique_ptr<Expression> initializer;
        std::vector<std::unique_ptr<AnnotationNode>> annotations;
        bool isFinal;

        VariableDeclaration() = default;
    };

    // Block Statement
    struct BlockStatement : public Statement {
        std::vector<std::unique_ptr<Statement>> statements;

        BlockStatement() = default;
    };

    // Expression Statement
    struct ExpressionStatement : public Statement {
        std::unique_ptr<Expression> expression;

        ExpressionStatement() = default;
    };

    // Return Statement
    struct ReturnStatement : public Statement {
        std::unique_ptr<Expression> value;

        ReturnStatement() = default;
    };

    // If Statement
    struct IfStatement : public Statement {
        std::unique_ptr<Expression> condition;
        std::unique_ptr<Statement> thenBranch;
        std::unique_ptr<Statement> elseBranch;

        IfStatement() = default;
    };

    // For Statement
    struct ForStatement : public Statement {
        std::unique_ptr<Statement> initializer;
        std::unique_ptr<Expression> condition;
        std::unique_ptr<Expression> increment;
        std::unique_ptr<Statement> body;

        ForStatement() = default;
    };

    // Echo Statement
    struct EchoStatement : public Statement {
        std::unique_ptr<Expression> value;

        EchoStatement() = default;
    };

    // Reset Statement
    struct ResetStatement : public Statement {
        std::unique_ptr<Expression> target;

        ResetStatement() = default;
    };

    // Measure Statement
    struct MeasureStatement : public Statement {
        std::unique_ptr<Expression> qubit;

        MeasureStatement() = default;
    };

    // Assignment
    struct AssignmentStatement : public Statement {
        std::string name;
        std::unique_ptr<Expression> value;

        AssignmentStatement() = default;
    };

    // Binary Expression
    struct BinaryExpression : public Expression {
        std::string op;
        std::unique_ptr<Expression> left;
        std::unique_ptr<Expression> right;

        BinaryExpression(const std::string& op, std::unique_ptr<Expression> left,
                         std::unique_ptr<Expression> right)
            : op(op), left(std::move(left)), right(std::move(right)) {}
    };

    // Unary Expression
    struct UnaryExpression : public Expression {
        std::string op;
        std::unique_ptr<Expression> right;

        UnaryExpression(const std::string& op, std::unique_ptr<Expression> right)
            : op(op), right(std::move(right)) {}
    };

    // Literal Expression
    struct LiteralExpression : public Expression {
        std::string value;

        LiteralExpression(const std::string& value) : value(value) {}
    };

    // Variable Expression
    struct VariableExpression : public Expression {
        std::string name;

        VariableExpression(const std::string& name) : name(name) {}
    };

    // Call Expression
    struct CallExpression : public Expression {
        std::unique_ptr<Expression> callee;
        std::vector<std::unique_ptr<Expression>> arguments;

        CallExpression(std::unique_ptr<Expression> callee,
                       std::vector<std::unique_ptr<Expression>> args)
            : callee(std::move(callee)), arguments(std::move(args)) {}
    };

    // Index Expression
    struct IndexExpression : public Expression {
        std::unique_ptr<Expression> collection;
        std::unique_ptr<Expression> index;

        IndexExpression() = default;
    };

    // Parenthesized Expression
    struct ParenthesizedExpression : public Expression {
        std::unique_ptr<Expression> expression;

        ParenthesizedExpression(std::unique_ptr<Expression> expr) : expression(std::move(expr)) {}
    };

    // Measure Expression
    struct MeasureExpression : public Expression {
        std::unique_ptr<Expression> qubit;

        MeasureExpression(std::unique_ptr<Expression> qubit) : qubit(std::move(qubit)) {}
    };

    // Assignment Expression
    struct AssignmentExpression : public Expression {
        std::string name;
        std::unique_ptr<Expression> value;

        AssignmentExpression(std::string name, std::unique_ptr<Expression> value)
            : name(std::move(name)), value(std::move(value)) {}
    };

    // Constructor Call Expression
    struct ConstructorCallExpression : public Expression {
        std::string className;
        std::vector<std::unique_ptr<Expression>> arguments;

        ConstructorCallExpression(const std::string& className,
                                  std::vector<std::unique_ptr<Expression>> args)
            : className(className), arguments(std::move(args)) {}
    };

    // Class Method Access Expression
    struct MemberAccessExpression : public Expression {
        std::unique_ptr<Expression> object;
        std::string member;

        MemberAccessExpression(std::unique_ptr<Expression> obj, const std::string& mem)
            : object(std::move(obj)), member(mem) {}
    };

    // Type Nodes
    struct PrimitiveType : public Type {
        std::string name;

        PrimitiveType(const std::string& name) : name(name) {}
    };

    struct LogicalType : public Type {
        std::string code;

        LogicalType(const std::string& code) : code(code) {}
    };

    struct ArrayType : public Type {
        std::unique_ptr<Type> elementType;

        ArrayType(std::unique_ptr<Type> elementType) : elementType(std::move(elementType)) {}
    };

    struct VoidType : public Type {
        VoidType() = default;
    };

    struct ObjectType : public Type {
        std::string className;

        ObjectType(const std::string& className) : className(className) {}
    };

    // Parameter
    struct Parameter : public ASTNode {
        std::string name;
        std::unique_ptr<Type> type;

        Parameter() = default;
    };

    // Annotation
    struct AnnotationNode : public ASTNode {
        std::string name;
        std::string value;

        AnnotationNode() = default;
        AnnotationNode(const std::string& name, const std::string& value)
            : name(name), value(value) {}
    };

    // Function Declaration
    struct FunctionDeclaration : public ASTNode {
        std::string name;
        std::vector<std::unique_ptr<Parameter>> params;
        std::unique_ptr<Type> returnType;
        std::unique_ptr<BlockStatement> body;
        std::vector<std::unique_ptr<AnnotationNode>> annotations;
        bool hasQuantumAnnotation;
        bool isConstructor = false;

        FunctionDeclaration() = default;
    };

    // Class Declaration
    struct ClassDeclaration : public ASTNode {
        std::string name;
        std::vector<std::unique_ptr<VariableDeclaration>> members;
        std::vector<std::unique_ptr<FunctionDeclaration>> methods;

        ClassDeclaration() = default;
    };

    // Program
    struct Program : public ASTNode {
        std::vector<std::unique_ptr<ImportStatement>> imports;
        std::vector<std::unique_ptr<FunctionDeclaration>> functions;
        std::vector<std::unique_ptr<ClassDeclaration>> classes;
        std::vector<std::unique_ptr<Statement>> statements;

        Program() = default;
    };
}