#pragma once

#include <memory>
#include <stdexcept>
#include <vector>

#include "../ast/ast.hpp"
#include "../lexer/token.hpp"

namespace bloch {
    // The Parser consumes a flat token stream and produces an AST.
    // It's a hand-written, recursive-descent parser tuned for clarity,
    // with small conveniences like a lookahead helper and an overflow
    // queue (m_extraStatements) for expanding multi-declarations.
    class Parser {
       public:
        explicit Parser(std::vector<Token> tokens);
        [[nodiscard]] std::unique_ptr<Program> parse();

       private:
        std::vector<Token> m_tokens;
        size_t m_current;
        // For multi-declarations (e.g. qubit a, b, c;), we parse the first
        // and stage the rest here, then flush them into the surrounding block.
        std::vector<std::unique_ptr<Statement>> m_extraStatements;

        // Token manipulation
        [[nodiscard]] const Token& peek() const;
        [[nodiscard]] const Token& previous() const;
        [[nodiscard]] const Token& advance();
        [[nodiscard]] const Token& expect(TokenType type, const std::string& message);

        // Token matching
        [[nodiscard]] bool match(TokenType type);
        [[nodiscard]] bool check(TokenType type) const;
        [[nodiscard]] bool checkNext(TokenType type) const;
        [[nodiscard]] bool checkFunctionAnnotation() const;
        [[nodiscard]] bool isAtEnd() const;

        void reportError(const std::string& msg);

        // Top level
        [[nodiscard]] std::unique_ptr<FunctionDeclaration> parseFunction();

        // Declarations
        [[nodiscard]] std::unique_ptr<VariableDeclaration> parseVariableDeclaration(
            bool isFinal, bool allowMultiple = true);
        [[nodiscard]] std::unique_ptr<VariableDeclaration> parseVariableDeclaration(
            std::unique_ptr<Type> preParsedType, bool isFinal, bool allowMultiple = true);
        [[nodiscard]] std::unique_ptr<AnnotationNode> parseAnnotation();
        [[nodiscard]] std::vector<std::unique_ptr<AnnotationNode>> parseAnnotations();

        // Statements
        [[nodiscard]] std::unique_ptr<Statement> parseStatement();
        [[nodiscard]] std::unique_ptr<BlockStatement> parseBlock();
        [[nodiscard]] std::unique_ptr<ReturnStatement> parseReturn();
        [[nodiscard]] std::unique_ptr<IfStatement> parseIf();
        [[nodiscard]] std::unique_ptr<ForStatement> parseFor();
        [[nodiscard]] std::unique_ptr<WhileStatement> parseWhile();
        [[nodiscard]] std::unique_ptr<EchoStatement> parseEcho();
        [[nodiscard]] std::unique_ptr<ResetStatement> parseReset();
        [[nodiscard]] std::unique_ptr<MeasureStatement> parseMeasure();
        [[nodiscard]] std::unique_ptr<AssignmentStatement> parseAssignment();
        [[nodiscard]] std::unique_ptr<ExpressionStatement> parseExpressionStatement();

        // Expressions
        [[nodiscard]] std::unique_ptr<Expression> parseExpression();
        [[nodiscard]] std::unique_ptr<Expression> parseAssignmentExpression();
        [[nodiscard]] std::unique_ptr<Expression> parseLogicalOr();
        [[nodiscard]] std::unique_ptr<Expression> parseLogicalAnd();
        [[nodiscard]] std::unique_ptr<Expression> parseBitwiseOr();
        [[nodiscard]] std::unique_ptr<Expression> parseBitwiseXor();
        [[nodiscard]] std::unique_ptr<Expression> parseBitwiseAnd();
        [[nodiscard]] std::unique_ptr<Expression> parseEquality();
        [[nodiscard]] std::unique_ptr<Expression> parseComparison();
        [[nodiscard]] std::unique_ptr<Expression> parseAdditive();
        [[nodiscard]] std::unique_ptr<Expression> parseMultiplicative();
        [[nodiscard]] std::unique_ptr<Expression> parseUnary();
        [[nodiscard]] std::unique_ptr<Expression> parseCall();
        [[nodiscard]] std::unique_ptr<Expression> parsePrimary();
        [[nodiscard]] std::unique_ptr<Expression> parseArrayLiteral();

        // Literals
        [[nodiscard]] std::unique_ptr<Expression> parseLiteral();

        // Types
        [[nodiscard]] std::unique_ptr<Type> parseType();
        [[nodiscard]] std::unique_ptr<Type> parsePrimitiveType();
        [[nodiscard]] std::unique_ptr<Type> parseArrayType(std::unique_ptr<Type> elementType,
                                                           int size = -1);

        // Parameters and Arguments
        [[nodiscard]] std::vector<std::unique_ptr<Parameter>> parseParameterList();
        [[nodiscard]] std::vector<std::unique_ptr<Expression>> parseArgumentList();

        // Helpers
        [[nodiscard]] std::unique_ptr<Type> cloneType(const Type& type);
        [[nodiscard]] std::vector<std::unique_ptr<AnnotationNode>> cloneAnnotations(
            const std::vector<std::unique_ptr<AnnotationNode>>& annotations);
        void flushExtraStatements(std::vector<std::unique_ptr<Statement>>& dest);
    };
}
