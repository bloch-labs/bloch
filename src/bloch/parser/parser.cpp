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

#include "parser.hpp"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include "../error/bloch_error.hpp"

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
        if (!checkNext(TokenType::Quantum))
            return false;
        return true;
    }

    bool Parser::isAtEnd() const { return peek().type == TokenType::Eof; }

    // Error
    void Parser::reportError(const std::string& msg) {
        const Token& token = peek();
        throw BlochError(ErrorCategory::Parse, token.line, token.column, msg);
    }

    // Main parse function
    std::unique_ptr<Program> Parser::parse() {
        auto program = std::make_unique<Program>();
        // We alternate between function declarations and top-level statements.
        // Extra statements generated from multi-declarations are flushed as we go.
        while (!isAtEnd()) {
            if (check(TokenType::Function) || checkFunctionAnnotation()) {
                program->functions.push_back(parseFunction());
            } else {
                program->statements.push_back(parseStatement());
                flushExtraStatements(program->statements);
            }
        }

        return program;
    }

    // Top level

    // Function Declaration
    std::unique_ptr<FunctionDeclaration> Parser::parseFunction() {
        auto func = std::make_unique<FunctionDeclaration>();

        // Parse annotations
        while (check(TokenType::At)) {
            (void)advance();
            if (match(TokenType::Quantum)) {
                std::string name = previous().value;
                std::string value = "";
                func->annotations.push_back(
                    std::make_unique<AnnotationNode>(AnnotationNode{name, value}));
                func->hasQuantumAnnotation = true;
            } else {
                const Token& invalid = peek();
                std::string invalidName = invalid.value.empty() ? std::string("") : invalid.value;
                reportError(std::string("\"") + "@" + invalidName +
                            "\" is not a valid Bloch annotation");
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
    std::unique_ptr<VariableDeclaration> Parser::parseVariableDeclaration(bool isFinal,
                                                                          bool allowMultiple) {
        return parseVariableDeclaration(nullptr, isFinal, allowMultiple);
    }

    std::unique_ptr<VariableDeclaration> Parser::parseVariableDeclaration(
        std::unique_ptr<Type> preParsedType, bool isFinal, bool allowMultiple) {
        auto var = std::make_unique<VariableDeclaration>();
        var->isFinal = isFinal;

        // Annotations
        var->annotations = parseAnnotations();
        for (auto& ann : var->annotations) {
            if (ann->name == "tracked")
                var->isTracked = true;
        }

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

        bool isQubitType = false;
        if (auto prim = dynamic_cast<PrimitiveType*>(var->varType.get()))
            isQubitType = prim->name == "qubit";

        bool hasInitializer = var->initializer != nullptr;
        // Support comma-separated qubit declarations (qubit a, b, c;).
        while (match(TokenType::Comma)) {
            if (!allowMultiple)
                reportError("Multiple declarations not allowed in this context");
            if (!isQubitType)
                reportError("only 'qubit' may be multi-declared");
            if (hasInitializer)
                reportError("Cannot initialise multiple qubit declarations");
            const Token& extraToken =
                expect(TokenType::Identifier, "Expected variable name after ','");
            auto extraVar = std::make_unique<VariableDeclaration>();
            extraVar->isFinal = isFinal;
            extraVar->annotations = cloneAnnotations(var->annotations);
            extraVar->varType = cloneType(*var->varType);
            extraVar->name = extraToken.value;
            extraVar->line = extraToken.line;
            extraVar->column = extraToken.column;
            m_extraStatements.push_back(std::move(extraVar));
        }

        (void)expect(TokenType::Semicolon, "expected ';' after declaration");

        return var;
    }

    // @quantum, @tracked
    std::unique_ptr<AnnotationNode> Parser::parseAnnotation() {
        (void)expect(TokenType::At, "Expected '@' to begin annotation");

        if (!check(TokenType::Quantum) && !check(TokenType::Tracked)) {
            const Token& invalid = peek();
            std::string invalidName = invalid.value.empty() ? std::string("") : invalid.value;
            reportError(std::string("\"") + "@" + invalidName +
                        "\" is not a valid Bloch annotation");
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
        if (match(TokenType::While))
            return parseWhile();
        if (match(TokenType::Echo))
            return parseEcho();
        if (match(TokenType::Reset))
            return parseReset();
        if (match(TokenType::Measure))
            return parseMeasure();

        if (check(TokenType::Identifier) && checkNext(TokenType::Equals))
            return parseAssignment();

        auto expr = parseExpression();
        if (match(TokenType::Question)) {
            auto thenBranch = parseStatement();
            (void)expect(TokenType::Colon, "Expected ':' after true branch");
            auto elseBranch = parseStatement();
            auto stmt = std::make_unique<TernaryStatement>();
            stmt->condition = std::move(expr);
            stmt->thenBranch = std::move(thenBranch);
            stmt->elseBranch = std::move(elseBranch);
            return stmt;
        }

        (void)expect(TokenType::Semicolon, "Expected ';' after expression");
        auto stmt = std::make_unique<ExpressionStatement>();
        stmt->expression = std::move(expr);
        return stmt;
    }

    // {...}
    std::unique_ptr<BlockStatement> Parser::parseBlock() {
        const Token& lbrace = expect(TokenType::LBrace, "Expected '{' to start block");

        auto block = std::make_unique<BlockStatement>();
        block->line = lbrace.line;
        block->column = lbrace.column;
        while (!check(TokenType::RBrace) && !isAtEnd()) {
            block->statements.push_back(parseStatement());
            flushExtraStatements(block->statements);
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
                initializer = parseVariableDeclaration(isFinal, false);
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

    // while (cond) {...}
    std::unique_ptr<WhileStatement> Parser::parseWhile() {
        (void)expect(TokenType::LParen, "Expected '(' after 'while'");
        auto condition = parseExpression();
        (void)expect(TokenType::RParen, "Expected ')' after condition");
        auto body = parseBlock();

        auto stmt = std::make_unique<WhileStatement>();
        stmt->condition = std::move(condition);
        stmt->body = std::move(body);
        return stmt;
    }

    // echo(expr);
    std::unique_ptr<EchoStatement> Parser::parseEcho() {
        const Token& echoTok = previous();
        (void)expect(TokenType::LParen, "Expected '(' after 'echo'");
        auto value = parseExpression();
        (void)expect(TokenType::RParen, "Expected ')' after echo argument");
        (void)expect(TokenType::Semicolon, "Expected ';' after echo statement");

        auto stmt = std::make_unique<EchoStatement>();
        stmt->value = std::move(value);
        stmt->line = echoTok.line;
        stmt->column = echoTok.column;
        return stmt;
    }

    // reset q0;
    std::unique_ptr<ResetStatement> Parser::parseReset() {
        const Token& resetTok = previous();
        auto stmt = std::make_unique<ResetStatement>();
        stmt->target = parseExpression();
        (void)expect(TokenType::Semicolon, "Expected ';' after reset target");
        stmt->line = resetTok.line;
        stmt->column = resetTok.column;
        return stmt;
    }

    // measure q0;
    std::unique_ptr<MeasureStatement> Parser::parseMeasure() {
        const Token& measureTok = previous();
        auto stmt = std::make_unique<MeasureStatement>();
        stmt->qubit = parseExpression();
        (void)expect(TokenType::Semicolon, "Expected ';' after measure target");
        stmt->line = measureTok.line;
        stmt->column = measureTok.column;
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
        auto expr = parseLogicalOr();

        if (match(TokenType::Equals)) {
            // Assignment to variable or array index
            if (auto varExpr = dynamic_cast<VariableExpression*>(expr.get())) {
                int line = varExpr->line;
                int column = varExpr->column;
                std::string name = varExpr->name;
                auto value = parseAssignmentExpression();
                auto assign = std::make_unique<AssignmentExpression>(name, std::move(value));
                assign->line = line;
                assign->column = column;
                return assign;
            } else if (auto idxExpr = dynamic_cast<IndexExpression*>(expr.get())) {
                // Take ownership of the IndexExpression to move its parts
                std::unique_ptr<IndexExpression> idx(static_cast<IndexExpression*>(expr.release()));
                int line = idx->line;
                int column = idx->column;
                auto value = parseAssignmentExpression();
                auto arrayAssign = std::make_unique<ArrayAssignmentExpression>(
                    std::move(idx->collection), std::move(idx->index), std::move(value));
                arrayAssign->line = line;
                arrayAssign->column = column;
                return arrayAssign;
            } else {
                reportError("Invalid assignment target");
            }
        }

        return expr;
    }

    std::unique_ptr<Expression> Parser::parseLogicalOr() {
        auto expr = parseLogicalAnd();

        while (match(TokenType::PipePipe)) {
            std::string op = previous().value;
            auto right = parseLogicalAnd();
            expr = std::make_unique<BinaryExpression>(
                BinaryExpression{op, std::move(expr), std::move(right)});
        }

        return expr;
    }

    std::unique_ptr<Expression> Parser::parseLogicalAnd() {
        auto expr = parseBitwiseOr();

        while (match(TokenType::AmpersandAmpersand)) {
            std::string op = previous().value;
            auto right = parseBitwiseOr();
            expr = std::make_unique<BinaryExpression>(
                BinaryExpression{op, std::move(expr), std::move(right)});
        }

        return expr;
    }

    std::unique_ptr<Expression> Parser::parseBitwiseOr() {
        auto expr = parseBitwiseXor();

        while (match(TokenType::Pipe)) {
            std::string op = previous().value;
            auto right = parseBitwiseXor();
            expr = std::make_unique<BinaryExpression>(
                BinaryExpression{op, std::move(expr), std::move(right)});
        }

        return expr;
    }

    std::unique_ptr<Expression> Parser::parseBitwiseXor() {
        auto expr = parseBitwiseAnd();

        while (match(TokenType::Caret)) {
            std::string op = previous().value;
            auto right = parseBitwiseAnd();
            expr = std::make_unique<BinaryExpression>(
                BinaryExpression{op, std::move(expr), std::move(right)});
        }

        return expr;
    }

    std::unique_ptr<Expression> Parser::parseBitwiseAnd() {
        auto expr = parseEquality();

        while (match(TokenType::Ampersand)) {
            std::string op = previous().value;
            auto right = parseEquality();
            expr = std::make_unique<BinaryExpression>(
                BinaryExpression{op, std::move(expr), std::move(right)});
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
        if (match(TokenType::Minus) || match(TokenType::Bang) || match(TokenType::Tilde)) {
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
                const Token& lparen = previous();
                std::vector<std::unique_ptr<Expression>> args;
                if (!check(TokenType::RParen)) {
                    do {
                        args.push_back(parseExpression());
                    } while (match(TokenType::Comma));
                }
                (void)expect(TokenType::RParen, "Expected ')' after arguments");
                int calleeLine = expr ? expr->line : 0;
                int calleeColumn = expr ? expr->column : 0;
                auto call =
                    std::make_unique<CallExpression>(std::move(expr), std::move(args));
                if (calleeLine > 0) {
                    call->line = calleeLine;
                    call->column = calleeColumn;
                } else {
                    call->line = lparen.line;
                    call->column = lparen.column;
                }
                expr = std::move(call);
            } else if (match(TokenType::LBracket)) {
                const Token& lbr = previous();
                auto idxExpr = std::make_unique<IndexExpression>();
                idxExpr->collection = std::move(expr);
                auto indexNode = parseExpression();
                // Parser check for constant negative indices like a[-1]
                bool negativeConst = false;
                if (auto lit = dynamic_cast<LiteralExpression*>(indexNode.get())) {
                    if (lit->literalType == "int") {
                        try {
                            int v = std::stoi(lit->value);
                            negativeConst = v < 0;
                        } catch (...) {
                        }
                    }
                } else if (auto unary = dynamic_cast<UnaryExpression*>(indexNode.get())) {
                    if (unary->op == "-") {
                        if (auto rlit = dynamic_cast<LiteralExpression*>(unary->right.get())) {
                            if (rlit->literalType == "int") {
                                try {
                                    int v = std::stoi(rlit->value);
                                    negativeConst = v > 0;  // -0 is allowed (treated as 0)
                                } catch (...) {
                                }
                            }
                        }
                    }
                }
                if (negativeConst) {
                    throw BlochError(ErrorCategory::Parse, lbr.line, lbr.column,
                                     "array index must be non-negative");
                }
                idxExpr->index = std::move(indexNode);
                (void)expect(TokenType::RBracket, "Expected ']' after index expression");
                idxExpr->line = lbr.line;
                idxExpr->column = lbr.column;
                expr = std::move(idxExpr);
            } else if (match(TokenType::PlusPlus) || match(TokenType::MinusMinus)) {
                std::string op = previous().value;
                auto post = std::make_unique<PostfixExpression>(op, std::move(expr));
                post->line = previous().line;
                post->column = previous().column;
                expr = std::move(post);
            } else {
                break;
            }
        }

        return expr;
    }

    std::unique_ptr<Expression> Parser::parsePrimary() {
        if (match(TokenType::IntegerLiteral) || match(TokenType::FloatLiteral) ||
            match(TokenType::BitLiteral) || match(TokenType::StringLiteral) ||
            match(TokenType::CharLiteral)) {
            auto tok = previous();
            std::string litType;
            switch (tok.type) {
                case TokenType::IntegerLiteral:
                    litType = "int";
                    break;
                case TokenType::FloatLiteral:
                    litType = "float";
                    break;
                case TokenType::BitLiteral:
                    litType = "bit";
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
            const Token& measureTok = previous();
            auto target = parseExpression();
            auto expr =
                std::make_unique<MeasureExpression>(MeasureExpression{std::move(target)});
            expr->line = measureTok.line;
            expr->column = measureTok.column;
            return expr;
        }

        if (match(TokenType::Identifier)) {
            const Token& token = previous();
            auto expr = std::make_unique<VariableExpression>(VariableExpression{token.value});
            expr->line = token.line;
            expr->column = token.column;
            return expr;
        }

        if (match(TokenType::LBrace)) {
            auto start = previous();
            auto expr = parseArrayLiteral();
            expr->line = start.line;
            expr->column = start.column;
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

    std::unique_ptr<Expression> Parser::parseArrayLiteral() {
        std::vector<std::unique_ptr<Expression>> elements;
        if (!check(TokenType::RBrace)) {
            do {
                elements.push_back(parseExpression());
            } while (match(TokenType::Comma));
        }
        (void)expect(TokenType::RBrace, "Expected '}' after array literal");
        return std::make_unique<ArrayLiteralExpression>(std::move(elements));
    }

    // Literals
    std::unique_ptr<Expression> Parser::parseLiteral() {
        const Token& token = advance();

        switch (token.type) {
            case TokenType::IntegerLiteral:
                return std::make_unique<LiteralExpression>(LiteralExpression{token.value, "int"});
            case TokenType::FloatLiteral:
                return std::make_unique<LiteralExpression>(LiteralExpression{token.value, "float"});
            case TokenType::BitLiteral:
                return std::make_unique<LiteralExpression>(LiteralExpression{token.value, "bit"});
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
                int arrSize = -1;
                if (!check(TokenType::RBracket)) {
                    if (!check(TokenType::IntegerLiteral))
                        reportError("Expected optional integer size in array type");
                    const Token& sizeTok = advance();
                    try {
                        arrSize = std::stoi(sizeTok.value);
                    } catch (...) {
                        reportError("Invalid integer size in array type");
                    }
                }
                (void)expect(TokenType::RBracket, "Expected ']' after '[' in array type");
                return parseArrayType(std::move(baseType), arrSize);
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

    std::unique_ptr<Type> Parser::parseArrayType(std::unique_ptr<Type> elementType, int size) {
        return std::make_unique<ArrayType>(std::move(elementType), size);
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

    std::unique_ptr<Type> Parser::cloneType(const Type& type) {
        if (auto prim = dynamic_cast<const PrimitiveType*>(&type))
            return std::make_unique<PrimitiveType>(prim->name);
        if (auto array = dynamic_cast<const ArrayType*>(&type))
            return std::make_unique<ArrayType>(cloneType(*array->elementType), array->size);
        if (dynamic_cast<const VoidType*>(&type))
            return std::make_unique<VoidType>();
        return nullptr;
    }

    std::vector<std::unique_ptr<AnnotationNode>> Parser::cloneAnnotations(
        const std::vector<std::unique_ptr<AnnotationNode>>& annotations) {
        std::vector<std::unique_ptr<AnnotationNode>> result;
        for (const auto& ann : annotations)
            result.push_back(std::make_unique<AnnotationNode>(ann->name, ann->value));
        return result;
    }

    void Parser::flushExtraStatements(std::vector<std::unique_ptr<Statement>>& dest) {
        for (auto& stmt : m_extraStatements) dest.push_back(std::move(stmt));
        m_extraStatements.clear();
    }
}
