#pragma once

#include <memory>
#include <string>
#include <vector>

namespace bloch {

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
    // (expr) — keeps source intent; may be useful for diagnostics.
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

    struct ArrayType : public Type {
        std::unique_ptr<Type> elementType;
        // Optional fixed size for the array. If negative, size is unspecified.
        int size = -1;

        ArrayType(std::unique_ptr<Type> elementType, int size = -1)
            : elementType(std::move(elementType)), size(size) {}
        void accept(ASTVisitor& visitor) override;
    };

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
        bool hasQuantumAnnotation = false;

        FunctionDeclaration() = default;
        void accept(ASTVisitor& visitor) override;
    };

    // Program
    struct Program : public ASTNode {
        std::vector<std::unique_ptr<FunctionDeclaration>> functions;
        std::vector<std::unique_ptr<Statement>> statements;

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
        virtual void visit(TernaryStatement& node) = 0;
        virtual void visit(AssignmentStatement& node) = 0;

        virtual void visit(BinaryExpression& node) = 0;
        virtual void visit(UnaryExpression& node) = 0;
        virtual void visit(PostfixExpression& node) = 0;
        virtual void visit(LiteralExpression& node) = 0;
        virtual void visit(VariableExpression& node) = 0;
        virtual void visit(CallExpression& node) = 0;
        virtual void visit(IndexExpression& node) = 0;
        virtual void visit(ArrayLiteralExpression& node) = 0;
        virtual void visit(ParenthesizedExpression& node) = 0;
        virtual void visit(MeasureExpression& node) = 0;
        virtual void visit(AssignmentExpression& node) = 0;
        virtual void visit(ArrayAssignmentExpression& node) = 0;

        virtual void visit(PrimitiveType& node) = 0;
        virtual void visit(ArrayType& node) = 0;
        virtual void visit(VoidType& node) = 0;

        virtual void visit(Parameter& node) = 0;
        virtual void visit(AnnotationNode& node) = 0;
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
    inline void TernaryStatement::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void AssignmentStatement::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void BinaryExpression::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void UnaryExpression::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void PostfixExpression::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void LiteralExpression::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void VariableExpression::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void CallExpression::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void IndexExpression::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void ArrayLiteralExpression::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void ParenthesizedExpression::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void MeasureExpression::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void AssignmentExpression::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void ArrayAssignmentExpression::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void PrimitiveType::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void ArrayType::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void VoidType::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void Parameter::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void AnnotationNode::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void FunctionDeclaration::accept(ASTVisitor& visitor) { visitor.visit(*this); }
    inline void Program::accept(ASTVisitor& visitor) { visitor.visit(*this); }
}
