#pragma once

#include <memory>
#include <string>
#include <vector>

namespace bloch {

    // Base Node Interfaces
    class ASTVisitor;  // Forward declaration

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

    // Import Statement
    struct ImportStatement : public Statement {
        std::string module;

        ImportStatement() = default;
        void accept(ASTVisitor& visitor) override;
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
        void accept(ASTVisitor& visitor) override;
    };

    // Block Statement
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

    // Echo Statement
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
    struct MeasureStatement : public Statement {
        std::unique_ptr<Expression> qubit;

        MeasureStatement() = default;
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

    // Literal Expression
    struct LiteralExpression : public Expression {
        std::string value;

        LiteralExpression(const std::string& value) : value(value) {}
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

    // Index Expression
    struct IndexExpression : public Expression {
        std::unique_ptr<Expression> collection;
        std::unique_ptr<Expression> index;

        IndexExpression() = default;
        void accept(ASTVisitor& visitor) override;
    };

    // Parenthesized Expression
    struct ParenthesizedExpression : public Expression {
        std::unique_ptr<Expression> expression;

        ParenthesizedExpression(std::unique_ptr<Expression> expr) : expression(std::move(expr)) {}
        void accept(ASTVisitor& visitor) override;
    };

    // Measure Expression
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

    // Constructor Call Expression
    struct ConstructorCallExpression : public Expression {
        std::string className;
        std::vector<std::unique_ptr<Expression>> arguments;

        ConstructorCallExpression(const std::string& className,
                                  std::vector<std::unique_ptr<Expression>> args)
            : className(className), arguments(std::move(args)) {}
        void accept(ASTVisitor& visitor) override;
    };

    // Class Method Access Expression
    struct MemberAccessExpression : public Expression {
        std::unique_ptr<Expression> object;
        std::string member;

        MemberAccessExpression(std::unique_ptr<Expression> obj, const std::string& mem)
            : object(std::move(obj)), member(mem) {}
        void accept(ASTVisitor& visitor) override;
    };

    // Type Nodes
    struct PrimitiveType : public Type {
        std::string name;

        PrimitiveType(const std::string& name) : name(name) {}
        void accept(ASTVisitor& visitor) override;
    };

    struct LogicalType : public Type {
        std::string code;

        LogicalType(const std::string& code) : code(code) {}
        void accept(ASTVisitor& visitor) override;
    };

    struct ArrayType : public Type {
        std::unique_ptr<Type> elementType;

        ArrayType(std::unique_ptr<Type> elementType) : elementType(std::move(elementType)) {}
        void accept(ASTVisitor& visitor) override;
    };

    struct VoidType : public Type {
        VoidType() = default;
        void accept(ASTVisitor& visitor) override;
    };

    struct ObjectType : public Type {
        std::string className;

        ObjectType(const std::string& className) : className(className) {}
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
        std::string value;

        AnnotationNode() = default;
        AnnotationNode(const std::string& name, const std::string& value)
            : name(name), value(value) {}
        void accept(ASTVisitor& visitor) override;
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
        void accept(ASTVisitor& visitor) override;
    };

    // Class Declaration
    struct ClassDeclaration : public ASTNode {
        std::string name;
        std::vector<std::unique_ptr<VariableDeclaration>> members;
        std::vector<std::unique_ptr<FunctionDeclaration>> methods;

        ClassDeclaration() = default;
        void accept(ASTVisitor& visitor) override;
    };

    // Program
    struct Program : public ASTNode {
        std::vector<std::unique_ptr<ImportStatement>> imports;
        std::vector<std::unique_ptr<FunctionDeclaration>> functions;
        std::vector<std::unique_ptr<ClassDeclaration>> classes;
        std::vector<std::unique_ptr<Statement>> statements;

        Program() = default;

        void accept(ASTVisitor& visitor) override;
    };

    // Visitor Interface
    class ASTVisitor {
       public:
        virtual ~ASTVisitor() = default;

        virtual void visit(ImportStatement& node) = 0;
        virtual void visit(VariableDeclaration& node) = 0;
        virtual void visit(BlockStatement& node) = 0;
        virtual void visit(ExpressionStatement& node) = 0;
        virtual void visit(ReturnStatement& node) = 0;
        virtual void visit(IfStatement& node) = 0;
        virtual void visit(ForStatement& node) = 0;
        virtual void visit(EchoStatement& node) = 0;
        virtual void visit(ResetStatement& node) = 0;
        virtual void visit(MeasureStatement& node) = 0;
        virtual void visit(AssignmentStatement& node) = 0;

        virtual void visit(BinaryExpression& node) = 0;
        virtual void visit(UnaryExpression& node) = 0;
        virtual void visit(LiteralExpression& node) = 0;
        virtual void visit(VariableExpression& node) = 0;
        virtual void visit(CallExpression& node) = 0;
        virtual void visit(IndexExpression& node) = 0;
        virtual void visit(ParenthesizedExpression& node) = 0;
        virtual void visit(MeasureExpression& node) = 0;
        virtual void visit(AssignmentExpression& node) = 0;
        virtual void visit(ConstructorCallExpression& node) = 0;
        virtual void visit(MemberAccessExpression& node) = 0;

        virtual void visit(PrimitiveType& node) = 0;
        virtual void visit(LogicalType& node) = 0;
        virtual void visit(ArrayType& node) = 0;
        virtual void visit(VoidType& node) = 0;
        virtual void visit(ObjectType& node) = 0;

        virtual void visit(Parameter& node) = 0;
        virtual void visit(AnnotationNode& node) = 0;
        virtual void visit(FunctionDeclaration& node) = 0;
        virtual void visit(ClassDeclaration& node) = 0;
        virtual void visit(Program& node) = 0;
    };

    // Inline accept implementations
    inline void ImportStatement::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void VariableDeclaration::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void BlockStatement::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void ExpressionStatement::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void ReturnStatement::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void IfStatement::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void ForStatement::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void EchoStatement::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void ResetStatement::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void MeasureStatement::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void AssignmentStatement::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void BinaryExpression::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void UnaryExpression::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void LiteralExpression::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void VariableExpression::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void CallExpression::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void IndexExpression::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void ParenthesizedExpression::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void MeasureExpression::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void AssignmentExpression::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void ConstructorCallExpression::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void MemberAccessExpression::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void PrimitiveType::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void LogicalType::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void ArrayType::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void VoidType::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void ObjectType::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void Parameter::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void AnnotationNode::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void FunctionDeclaration::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void ClassDeclaration::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void Program::accept(ASTVisitor& visitor) { visitor.visit(*this); }
}