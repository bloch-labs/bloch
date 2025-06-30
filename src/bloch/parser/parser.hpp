#pragma once

#include <memory>
#include <stdexcept>
#include <vector>

#include "../ast/ast.hpp"
#include "../lexer/token.hpp"

namespace bloch {
    class Parser {
       public:
        explicit Parser(const std::vector<Token>& tokens);
        std::unique_ptr<Program> parse();

       private:
        const std::vector<Token>& m_tokens;
        size_t m_current;

        // Token manipulation
        const Token& peek() const;
        const Token& previous() const;
        const Token& advance();
        const Token& expect(TokenType type, const std::string& message);

        // Token matching
        bool match(TokenType type);
        bool check(TokenType type) const;
        bool checkNext(TokenType type) const;
        bool checkFunctionAnnotation() const;
        bool isAtEnd() const;

        void reportError(const std::string& msg);

        // Top level
        std::unique_ptr<ImportStatement> parseImport();
        std::unique_ptr<FunctionDeclaration> parseFunction();
        std::unique_ptr<ClassDeclaration> parseClass();
        std::unique_ptr<Statement> parseTopLevelStatement();

        // Declarations
        std::unique_ptr<VariableDeclaration> parseVariableDeclaration(bool isFinal);
        std::unique_ptr<VariableDeclaration> parseVariableDeclaration(
            std::unique_ptr<Type> preParsedType, bool isFinal);
        std::unique_ptr<AnnotationNode> parseAnnotation();
        std::vector<std::unique_ptr<AnnotationNode>> parseAnnotations();

        // Statements
        std::unique_ptr<Statement> parseStatement();
        std::unique_ptr<BlockStatement> parseBlock();
        std::unique_ptr<ReturnStatement> parseReturn();
        std::unique_ptr<IfStatement> parseIf();
        std::unique_ptr<ForStatement> parseFor();
        std::unique_ptr<EchoStatement> parseEcho();
        std::unique_ptr<ResetStatement> parseReset();
        std::unique_ptr<MeasureStatement> parseMeasure();
        std::unique_ptr<AssignmentStatement> parseAssignment();
        std::unique_ptr<ExpressionStatement> parseExpressionStatement();

        // Expressions
        std::unique_ptr<Expression> parseExpression();
        std::unique_ptr<Expression> parseAssignmentExpression();
        std::unique_ptr<Expression> parseComparison();
        std::unique_ptr<Expression> parseAdditive();
        std::unique_ptr<Expression> parseMultiplicative();
        std::unique_ptr<Expression> parseUnary();
        std::unique_ptr<Expression> parseCall();
        std::unique_ptr<Expression> parsePrimary();

        // Literals
        std::unique_ptr<Expression> parseLiteral();

        // Types
        std::unique_ptr<Type> parseType();
        std::unique_ptr<Type> parsePrimitiveType();
        std::unique_ptr<Type> parseArrayType(std::unique_ptr<Type> elementType);

        // Parameters and Arguments
        std::vector<std::unique_ptr<Parameter>> parseParameterList();
        std::vector<std::unique_ptr<Expression>> parseArgumentList();
    };
}
