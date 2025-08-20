#include "parser.hpp"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include "../error/bloch_runtime_error.hpp"

namespace bloch {
    Parser::Parser(std::vector<Token> tokens) : m_tokens(std::move(tokens)), m_current(0) {}

    // Token manipulation
    const Token& Parser::peek() const {
        if (m_current >= m_tokens.size())
            return m_tokens.back();
        return m_tokens[m_current];
    }
    const Token& Parser::previous() const { return m_tokens[m_current - 1]; }
    const Token& Parser::advance() {
        if (!isAtEnd())
            m_current++;
        return previous();
    }
    const Token& Parser::expect(TokenType type, const std::string& message) {
        if (check(type))
            return advance();
        reportError(message);
        return peek();
    }

    // Token matching
    bool Parser::match(TokenType type) {
        if (check(type)) {
            (void)advance();
            return true;
        }
        return false;
    }
    bool Parser::check(TokenType type) const {
        if (isAtEnd())
            return false;
        return peek().type == type;
    }
    bool Parser::checkNext(TokenType type) const {
        if (m_current + 1 >= m_tokens.size())
            return false;
        return m_tokens[m_current + 1].type == type;
    }
    bool Parser::checkFunctionAnnotation() const {
        if (!check(TokenType::At))
            return false;
        if (!checkNext(TokenType::Quantum) && !checkNext(TokenType::Adjoint))
            return false;
        return true;
    }

    bool Parser::isAtEnd() const { return peek().type == TokenType::Eof; }

    // Error
    void Parser::reportError(const std::string& msg) {
        const Token& token = peek();
        throw BlochRuntimeError("Bloch Parser Error", token.line, token.column, msg);
    }

    // Main parse function
    std::unique_ptr<Program> Parser::parse() {
        auto program = std::make_unique<Program>();

        while (!isAtEnd()) {
            if (check(TokenType::Import)) {
                program->imports.push_back(parseImport());
            } else if (check(TokenType::Function) || checkFunctionAnnotation()) {
                program->functions.push_back(parseFunction());
            } else {
                program->statements.push_back(parseStatement());
            }
        }

        return program;
    }

    // Top level

    // import module;
    std::unique_ptr<ImportStatement> Parser::parseImport() {
        (void)expect(TokenType::Import, "Expected 'import' keyword");

        auto stmt = std::make_unique<ImportStatement>();
        const Token& moduleToken =
            expect(TokenType::Identifier, "Expected module name after 'import'");
        stmt->module = moduleToken.value;
        (void)expect(TokenType::Semicolon, "Expected ';' after import statement");

        return stmt;
    }

    // Function Declaration
    std::unique_ptr<FunctionDeclaration> Parser::parseFunction() {
        auto func = std::make_unique<FunctionDeclaration>();

        // Parse annotations
        while (check(TokenType::At)) {
            (void)advance();
            if (match(TokenType::Quantum) || match(TokenType::Adjoint)) {
                std::string name = previous().value;
                std::string value = "";
                func->annotations.push_back(
                    std::make_unique<AnnotationNode>(AnnotationNode{name, value}));
                func->hasQuantumAnnotation = true;
            } else {
                reportError("Expected annotation name after '@'");
            }
        }

        (void)expect(TokenType::Function, "Expected 'function' keyword");

        if (!check(TokenType::Identifier)) {
            reportError("Expected function name after 'function' keyword");
        }
        const Token& nameToken = advance();
        func->name = nameToken.value;
        func->line = nameToken.line;
        func->column = nameToken.column;

        // Parse parameters
        (void)expect(TokenType::LParen, "Expected '(' after function name");
        while (!check(TokenType::RParen)) {
            auto param = std::make_unique<Parameter>();

            param->type = parseType();

            const Token& paramToken = expect(TokenType::Identifier, "Expected parameter name");
            param->name = paramToken.value;
            param->line = paramToken.line;
            param->column = paramToken.column;

            func->params.push_back(std::move(param));

            if (!match(TokenType::Comma))
                break;
        }
        (void)expect(TokenType::RParen, "Expected ')' after parameters");

        // Return type
        (void)expect(TokenType::Arrow, "Expected '->' before return type");

        func->returnType = parseType();

        // Body
        func->body = parseBlock();

        return func;
    }

    // Declarations
    std::unique_ptr<VariableDeclaration> Parser::parseVariableDeclaration(bool isFinal) {
        return parseVariableDeclaration(nullptr, isFinal);
    }

    std::unique_ptr<VariableDeclaration> Parser::parseVariableDeclaration(
        std::unique_ptr<Type> preParsedType, bool isFinal) {
        auto var = std::make_unique<VariableDeclaration>();
        var->isFinal = isFinal;

        // Annotations
        var->annotations = parseAnnotations();

        if (preParsedType) {
            var->varType = std::move(preParsedType);
        } else {
            var->varType = parseType();
        }

        // Name
        if (!check(TokenType::Identifier)) {
            reportError("Expected variable name");
        }
        const Token& variableToken = advance();
        var->name = variableToken.value;
        var->line = variableToken.line;
        var->column = variableToken.column;

        // Initializer
        if (match(TokenType::Equals)) {
            var->initializer = parseExpression();
        }

        (void)expect(TokenType::Semicolon, "Expected ';' after variable declaration");

        return var;
    }

    // @quantum, @adjoint
    std::unique_ptr<AnnotationNode> Parser::parseAnnotation() {
        (void)expect(TokenType::At, "Expected '@' to begin annotation");

        if (!check(TokenType::Quantum) && !check(TokenType::Adjoint)) {
            reportError("Unknown annotation");
        }

        auto nameToken = advance();
        auto annotation = std::make_unique<AnnotationNode>();
        annotation->name = nameToken.value;
        return annotation;
    }

    std::vector<std::unique_ptr<AnnotationNode>> Parser::parseAnnotations() {
        std::vector<std::unique_ptr<AnnotationNode>> annotations;

        while (check(TokenType::At)) {
            annotations.push_back(parseAnnotation());
        }

        return annotations;
    }

    // Statements

    std::unique_ptr<Statement> Parser::parseStatement() {
        if (check(TokenType::LBrace))
            return parseBlock();

        bool isFinal = match(TokenType::Final);

        // Lookahead for user-defined type declarations: Identifier Identifier
        if (check(TokenType::Identifier) && checkNext(TokenType::Identifier)) {
            auto type = parseType();  // consumes the type name (first identifier)
            return parseVariableDeclaration(std::move(type), isFinal);
        }

        // Match primitive declarations or annotated declarations
        if (check(TokenType::At) || check(TokenType::Int) || check(TokenType::Float) ||
            check(TokenType::Char) || check(TokenType::String) || check(TokenType::Bit) ||
            check(TokenType::Qubit)) {
            return parseVariableDeclaration(isFinal);
        }

        // Standard statements
        if (match(TokenType::Return))
            return parseReturn();
        if (match(TokenType::If))
            return parseIf();
        if (match(TokenType::For))
            return parseFor();
        if (match(TokenType::Echo))
            return parseEcho();
        if (match(TokenType::Reset))
            return parseReset();
        if (match(TokenType::Measure))
            return parseMeasure();

        if (check(TokenType::Identifier) && checkNext(TokenType::Equals))
            return parseAssignment();

        return parseExpressionStatement();
    }

    // {...}
    std::unique_ptr<BlockStatement> Parser::parseBlock() {
        const Token& lbrace = expect(TokenType::LBrace, "Expected '{' to start block");

        auto block = std::make_unique<BlockStatement>();
        block->line = lbrace.line;
        block->column = lbrace.column;
        while (!check(TokenType::RBrace) && !isAtEnd()) {
            block->statements.push_back(parseStatement());
        }

        (void)expect(TokenType::RBrace, "Expected '}' to end block");
        return block;
    }

    // return expr;
    std::unique_ptr<ReturnStatement> Parser::parseReturn() {
        auto stmt = std::make_unique<ReturnStatement>();
        stmt->line = previous().line;
        stmt->column = previous().column;

        if (!check(TokenType::Semicolon)) {
            stmt->value = parseExpression();
        }

        (void)expect(TokenType::Semicolon, "Expected ';' after return value");
        return stmt;
    }

    // if (cond) {...} else {...}
    std::unique_ptr<IfStatement> Parser::parseIf() {
        (void)expect(TokenType::LParen, "Expected '(' after 'if'");
        auto condition = parseExpression();
        (void)expect(TokenType::RParen, "Expected ')' after condition");

        auto thenBranch = parseBlock();

        std::unique_ptr<BlockStatement> elseBranch = nullptr;
        if (match(TokenType::Else)) {
            elseBranch = parseBlock();
        }

        auto stmt = std::make_unique<IfStatement>();
        stmt->condition = std::move(condition);
        stmt->thenBranch = std::move(thenBranch);
        stmt->elseBranch = std::move(elseBranch);
        return stmt;
    }

    // for (init; cond; update) {...}
    std::unique_ptr<ForStatement> Parser::parseFor() {
        (void)expect(TokenType::LParen, "Expected '(' after 'for'");

        std::unique_ptr<Statement> initializer = nullptr;

        if (!check(TokenType::Semicolon)) {
            bool isFinal = match(TokenType::Final);

            if (check(TokenType::Int) || check(TokenType::Float) || check(TokenType::Char) ||
                check(TokenType::String) || check(TokenType::Bit) || check(TokenType::Qubit)) {
                initializer = parseVariableDeclaration(isFinal);
            } else {
                if (isFinal) {
                    reportError("Expected variable type after 'final'");
                }
                initializer = parseExpressionStatement();
            }
        } else {
            (void)advance();
        }

        auto condition = parseExpression();
        (void)expect(TokenType::Semicolon, "Expected ';' after loop condition");

        auto increment = parseExpression();
        (void)expect(TokenType::RParen, "Expected ')' after for clause");

        auto body = parseBlock();

        auto stmt = std::make_unique<ForStatement>();
        stmt->initializer = std::move(initializer);
        stmt->condition = std::move(condition);
        stmt->increment = std::move(increment);
        stmt->body = std::move(body);

        return stmt;
    }

    // echo(expr);
    std::unique_ptr<EchoStatement> Parser::parseEcho() {
        (void)expect(TokenType::LParen, "Expected '(' after 'echo'");
        auto value = parseExpression();
        (void)expect(TokenType::RParen, "Expected ')' after echo argument");
        (void)expect(TokenType::Semicolon, "Expected ';' after echo statement");

        auto stmt = std::make_unique<EchoStatement>();
        stmt->value = std::move(value);
        return stmt;
    }

    // reset q0;
    std::unique_ptr<ResetStatement> Parser::parseReset() {
        auto stmt = std::make_unique<ResetStatement>();
        stmt->target = parseExpression();
        (void)expect(TokenType::Semicolon, "Expected ';' after reset target");
        return stmt;
    }

    // measure q0;
    std::unique_ptr<MeasureStatement> Parser::parseMeasure() {
        auto stmt = std::make_unique<MeasureStatement>();
        stmt->qubit = parseExpression();
        (void)expect(TokenType::Semicolon, "Expected ';' after measure target");
        return stmt;
    }

    // x = expr;
    std::unique_ptr<AssignmentStatement> Parser::parseAssignment() {
        if (!check(TokenType::Identifier)) {
            reportError("Expected variable name in assignment");
        }

        const Token& nameToken = advance();
        std::string name = nameToken.value;
        (void)expect(TokenType::Equals, "Expected '=' in assignment");

        auto stmt = std::make_unique<AssignmentStatement>();
        stmt->name = name;
        stmt->line = nameToken.line;
        stmt->column = nameToken.column;
        stmt->value = parseExpression();

        (void)expect(TokenType::Semicolon, "Expected ';' after assignment");
        return stmt;
    }

    std::unique_ptr<ExpressionStatement> Parser::parseExpressionStatement() {
        auto expr = parseExpression();
        (void)expect(TokenType::Semicolon, "Expected ';' after expression");
        auto stmt = std::make_unique<ExpressionStatement>();
        stmt->expression = std::move(expr);
        return stmt;
    }

    // Expressions

    std::unique_ptr<Expression> Parser::parseExpression() { return parseAssignmentExpression(); }

    std::unique_ptr<Expression> Parser::parseAssignmentExpression() {
        auto expr = parseEquality();

        if (match(TokenType::Equals)) {
            // Must be a variable on the left-hand side
            if (auto varExpr = dynamic_cast<VariableExpression*>(expr.get())) {
                int line = varExpr->line;
                int column = varExpr->column;
                std::string name = varExpr->name;
                auto value = parseAssignmentExpression();
                auto assign = std::make_unique<AssignmentExpression>(name, std::move(value));
                assign->line = line;
                assign->column = column;
                return assign;
            } else {
                reportError("Invalid assignment target");
            }
        }

        return expr;
    }

    std::unique_ptr<Expression> Parser::parseEquality() {
        auto expr = parseComparison();

        while (match(TokenType::EqualEqual) || match(TokenType::BangEqual)) {
            std::string op = previous().value;
            auto right = parseComparison();
            expr = std::make_unique<BinaryExpression>(
                BinaryExpression{op, std::move(expr), std::move(right)});
        }

        return expr;
    }

    std::unique_ptr<Expression> Parser::parseComparison() {
        auto expr = parseAdditive();

        while (match(TokenType::Greater) || match(TokenType::Less) ||
               match(TokenType::GreaterEqual) || match(TokenType::LessEqual)) {
            std::string op = previous().value;
            auto right = parseAdditive();
            expr = std::make_unique<BinaryExpression>(
                BinaryExpression{op, std::move(expr), std::move(right)});
        }

        return expr;
    }

    std::unique_ptr<Expression> Parser::parseAdditive() {
        auto expr = parseMultiplicative();

        while (match(TokenType::Plus) || match(TokenType::Minus)) {
            std::string op = previous().value;
            auto right = parseMultiplicative();
            expr = std::make_unique<BinaryExpression>(
                BinaryExpression{op, std::move(expr), std::move(right)});
        }

        return expr;
    }

    std::unique_ptr<Expression> Parser::parseMultiplicative() {
        auto expr = parseUnary();

        while (match(TokenType::Star) || match(TokenType::Slash) || match(TokenType::Percent)) {
            std::string op = previous().value;
            auto right = parseUnary();
            expr = std::make_unique<BinaryExpression>(
                BinaryExpression{op, std::move(expr), std::move(right)});
        }

        return expr;
    }

    std::unique_ptr<Expression> Parser::parseUnary() {
        if (match(TokenType::Minus)) {
            std::string op = previous().value;
            auto right = parseUnary();
            return std::make_unique<UnaryExpression>(UnaryExpression{op, std::move(right)});
        }

        return parseCall();
    }

    std::unique_ptr<Expression> Parser::parseCall() {
        auto expr = parsePrimary();

        while (true) {
            if (match(TokenType::LParen)) {
                std::vector<std::unique_ptr<Expression>> args;
                if (!check(TokenType::RParen)) {
                    do {
                        args.push_back(parseExpression());
                    } while (match(TokenType::Comma));
                }
                (void)expect(TokenType::RParen, "Expected ')' after arguments");
                expr = std::make_unique<CallExpression>(std::move(expr), std::move(args));
            } else {
                break;
            }
        }

        return expr;
    }

    std::unique_ptr<Expression> Parser::parsePrimary() {
        if (match(TokenType::IntegerLiteral) || match(TokenType::FloatLiteral) ||
            match(TokenType::StringLiteral) || match(TokenType::CharLiteral)) {
            auto tok = previous();
            std::string litType;
            switch (tok.type) {
                case TokenType::IntegerLiteral:
                    litType = "int";
                    break;
                case TokenType::FloatLiteral:
                    litType = "float";
                    break;
                case TokenType::CharLiteral:
                    litType = "char";
                    break;
                case TokenType::StringLiteral:
                    litType = "string";
                    break;
                default:
                    break;
            }
            return std::make_unique<LiteralExpression>(LiteralExpression{tok.value, litType});
        }

        if (match(TokenType::Measure)) {
            auto target = parseExpression();
            return std::make_unique<MeasureExpression>(MeasureExpression{std::move(target)});
        }

        if (match(TokenType::Identifier)) {
            const Token& token = previous();
            auto expr = std::make_unique<VariableExpression>(VariableExpression{token.value});
            expr->line = token.line;
            expr->column = token.column;
            return expr;
        }

        if (match(TokenType::LParen)) {
            auto expr = parseExpression();
            (void)expect(TokenType::RParen, "Expected ')' after expression");
            return std::make_unique<ParenthesizedExpression>(
                ParenthesizedExpression{std::move(expr)});
        }

        reportError("Expected expression");
        return nullptr;
    }

    // Literals
    std::unique_ptr<Expression> Parser::parseLiteral() {
        const Token& token = advance();

        switch (token.type) {
            case TokenType::IntegerLiteral:
                return std::make_unique<LiteralExpression>(LiteralExpression{token.value, "int"});
            case TokenType::FloatLiteral:
                return std::make_unique<LiteralExpression>(LiteralExpression{token.value, "float"});
            case TokenType::CharLiteral:
                return std::make_unique<LiteralExpression>(LiteralExpression{token.value, "char"});
            case TokenType::StringLiteral:
                return std::make_unique<LiteralExpression>(
                    LiteralExpression{token.value, "string"});
            default:
                reportError("Expected a literal value.");
                return nullptr;
        }
    }

    // Types

    std::unique_ptr<Type> Parser::parseType() {
        // First: handle primitives
        if (check(TokenType::Void) || check(TokenType::Int) || check(TokenType::Float) ||
            check(TokenType::Char) || check(TokenType::String) || check(TokenType::Bit) ||
            check(TokenType::Qubit)) {
            auto baseType = parsePrimitiveType();

            // Array types are only allowed for primitive types
            if (match(TokenType::LBracket)) {
                (void)expect(TokenType::RBracket, "Expected ']' after '[' in array type");
                return parseArrayType(std::move(baseType));
            }

            return baseType;
        }

        reportError("Expected type");
        return nullptr;
    }

    std::unique_ptr<Type> Parser::parsePrimitiveType() {
        if (check(TokenType::Void)) {
            (void)advance();
            return std::make_unique<VoidType>();
        }

        if (check(TokenType::Int) || check(TokenType::Float) || check(TokenType::Char) ||
            check(TokenType::String) || check(TokenType::Bit) || check(TokenType::Qubit)) {
            std::string typeName = advance().value;
            return std::make_unique<PrimitiveType>(typeName);
        }

        reportError("Expected primitive type");
        return nullptr;
    }

    std::unique_ptr<Type> Parser::parseArrayType(std::unique_ptr<Type> elementType) {
        return std::make_unique<ArrayType>(std::move(elementType));
    }

    // Parameters and Argments
    std::vector<std::unique_ptr<Parameter>> Parser::parseParameterList() {
        std::vector<std::unique_ptr<Parameter>> parameters;

        while (!check(TokenType::RParen)) {
            auto param = std::make_unique<Parameter>();

            // Parse type
            param->type = parseType();

            // Parse name
            if (!check(TokenType::Identifier)) {
                reportError("Expected parameter name.");
            }
            param->name = advance().value;

            parameters.push_back(std::move(param));

            if (!match(TokenType::Comma))
                break;
        }

        return parameters;
    }

    std::vector<std::unique_ptr<Expression>> Parser::parseArgumentList() {
        std::vector<std::unique_ptr<Expression>> args;

        if (check(TokenType::RParen)) {
            return args;
        }

        do {
            args.push_back(parseExpression());
        } while (match(TokenType::Comma));

        return args;
    }
}