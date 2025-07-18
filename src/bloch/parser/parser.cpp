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
            advance();
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
            } else if (check(TokenType::Class)) {
                program->classes.push_back(parseClass());
            } else {
                program->statements.push_back(parseStatement());
            }
        }

        return program;
    }

    // Top level

    // import module;
    std::unique_ptr<ImportStatement> Parser::parseImport() {
        expect(TokenType::Import, "Expected 'import' keyword");

        auto stmt = std::make_unique<ImportStatement>();
        if (!check(TokenType::Identifier)) {
            reportError("Expected module name after 'import'");
        }

        stmt->module = advance().value;
        expect(TokenType::Semicolon, "Expected ';' after import statement");

        return stmt;
    }

    // Function Declaration
    std::unique_ptr<FunctionDeclaration> Parser::parseFunction() {
        auto func = std::make_unique<FunctionDeclaration>();

        // Parse annotations
        while (check(TokenType::At)) {
            advance();
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

        expect(TokenType::Function, "Expected 'function' keyword");

        // Constructor detection
        if (match(TokenType::Star)) {
            func->isConstructor = true;

            if (!check(TokenType::Identifier)) {
                reportError("Expected constructor name after '*'");
            }

            const Token& nameToken = advance();
            func->name = nameToken.value;
            func->line = nameToken.line;
            func->column = nameToken.column;
        } else {
            func->isConstructor = false;

            if (!check(TokenType::Identifier)) {
                reportError("Expected function name after 'function' keyword");
            }
            const Token& nameToken = advance();
            func->name = nameToken.value;
            func->line = nameToken.line;
            func->column = nameToken.column;
        }

        // Parse parameters
        expect(TokenType::LParen, "Expected '(' after function name");
        while (!check(TokenType::RParen)) {
            auto param = std::make_unique<Parameter>();

            param->type = parseType();

            if (!check(TokenType::Identifier)) {
                reportError("Expected parameter name");
            }
            const Token& paramToken = advance();
            param->name = paramToken.value;
            param->line = paramToken.line;
            param->column = paramToken.column;

            func->params.push_back(std::move(param));

            if (!match(TokenType::Comma))
                break;
        }
        expect(TokenType::RParen, "Expected ')' after parameters");

        // Return type
        expect(TokenType::Arrow, "Expected '->' before return type");

        func->returnType = parseType();

        // Body
        func->body = parseBlock();

        return func;
    }

    std::unique_ptr<ClassDeclaration> Parser::parseClass() {
        expect(TokenType::Class, "Expected 'class' keyword");

        auto clazz = std::make_unique<ClassDeclaration>();

        if (!check(TokenType::Identifier)) {
            reportError("Expected class name after 'class'");
        }
        clazz->name = advance().value;

        expect(TokenType::LBrace, "Expected '{' to start class body");

        while (!check(TokenType::RBrace) && !isAtEnd()) {
            // @members("public") or @members("private"):
            if (check(TokenType::At) && checkNext(TokenType::Members)) {
                advance();
                advance();

                expect(TokenType::LParen, "Expected '(' after @members");
                if (!check(TokenType::StringLiteral)) {
                    reportError("Expected access modifier string in @members");
                }

                std::string accessModifier = advance().value;
                if (accessModifier != "\"public\"" && accessModifier != "\"private\"") {
                    reportError("Access modifier must be \"public\" or \"private\"");
                }
                accessModifier = accessModifier.substr(1, accessModifier.length() - 2);

                expect(TokenType::RParen, "Expected ')' after access modifier");
                expect(TokenType::Colon, "Expected ':' after @members(...)");

                while (!check(TokenType::At) && !check(TokenType::RBrace)) {
                    bool isFinal = match(TokenType::Final);
                    auto member = parseVariableDeclaration(isFinal);
                    member->access = accessModifier;
                    clazz->members.push_back(std::move(member));
                }

                // @methods:
            } else if (check(TokenType::At) && checkNext(TokenType::Methods)) {
                advance();
                advance();

                expect(TokenType::Colon, "Expected ':' after @methods");

                while (!check(TokenType::At) && !check(TokenType::RBrace)) {
                    clazz->methods.push_back(parseFunction());
                }

            } else {
                reportError("Only @members(...) or @methods are allowed inside class body");
            }
        }

        expect(TokenType::RBrace, "Expected '}' to end class body");
        return clazz;
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

        expect(TokenType::Semicolon, "Expected ';' after variable declaration");

        return var;
    }

    // @quantum, @adjoint, @state
    std::unique_ptr<AnnotationNode> Parser::parseAnnotation() {
        expect(TokenType::At, "Expected '@' to begin annotation");

        if (!check(TokenType::Quantum) && !check(TokenType::Adjoint) && !check(TokenType::State)) {
            reportError("Unknown annotation");
        }

        auto nameToken = advance();
        auto annotation = std::make_unique<AnnotationNode>();
        annotation->name = nameToken.value;

        if (annotation->name == "state") {
            expect(TokenType::LParen, "Expected '(' after @state");
            if (!check(TokenType::CharLiteral) && !check(TokenType::StringLiteral)) {
                reportError("Expected character or string inside @state(...)");
            }
            annotation->value = advance().value;
            expect(TokenType::RParen, "Expected ')' after @state argument");
        }

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

        expect(TokenType::RBrace, "Expected '}' to end block");
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

        expect(TokenType::Semicolon, "Expected ';' after return value");
        return stmt;
    }

    // if (cond) {...} else {...}
    std::unique_ptr<IfStatement> Parser::parseIf() {
        expect(TokenType::LParen, "Expected '(' after 'if'");
        auto condition = parseExpression();
        expect(TokenType::RParen, "Expected ')' after condition");

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
        expect(TokenType::LParen, "Expected '(' after 'for'");

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
            advance();
        }

        auto condition = parseExpression();
        expect(TokenType::Semicolon, "Expected ';' after loop condition");

        auto increment = parseExpression();
        expect(TokenType::RParen, "Expected ')' after for clause");

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
        expect(TokenType::LParen, "Expected '(' after 'echo'");
        auto value = parseExpression();
        expect(TokenType::RParen, "Expected ')' after echo argument");
        expect(TokenType::Semicolon, "Expected ';' after echo statement");

        auto stmt = std::make_unique<EchoStatement>();
        stmt->value = std::move(value);
        return stmt;
    }

    // reset q0;
    std::unique_ptr<ResetStatement> Parser::parseReset() {
        auto stmt = std::make_unique<ResetStatement>();
        stmt->target = parseExpression();
        expect(TokenType::Semicolon, "Expected ';' after reset target");
        return stmt;
    }

    // measure q0;
    std::unique_ptr<MeasureStatement> Parser::parseMeasure() {
        auto stmt = std::make_unique<MeasureStatement>();
        stmt->qubit = parseExpression();
        expect(TokenType::Semicolon, "Expected ';' after measure target");
        return stmt;
    }

    // x = expr;
    std::unique_ptr<AssignmentStatement> Parser::parseAssignment() {
        if (!check(TokenType::Identifier)) {
            reportError("Expected variable name in assignment");
        }

        const Token& nameToken = advance();
        std::string name = nameToken.value;
        expect(TokenType::Equals, "Expected '=' in assignment");

        auto stmt = std::make_unique<AssignmentStatement>();
        stmt->name = name;
        stmt->line = nameToken.line;
        stmt->column = nameToken.column;
        stmt->value = parseExpression();

        expect(TokenType::Semicolon, "Expected ';' after assignment");
        return stmt;
    }

    std::unique_ptr<ExpressionStatement> Parser::parseExpressionStatement() {
        auto expr = parseExpression();
        expect(TokenType::Semicolon, "Expected ';' after expression");
        auto stmt = std::make_unique<ExpressionStatement>();
        stmt->expression = std::move(expr);
        return stmt;
    }

    // Expressions

    std::unique_ptr<Expression> Parser::parseExpression() { return parseAssignmentExpression(); }

    std::unique_ptr<Expression> Parser::parseAssignmentExpression() {
        auto expr = parseEquality();

        if (match(TokenType::Equals)) {
            auto equals = previous();

            // Must be a variable on the left-hand side
            if (auto varExpr = dynamic_cast<VariableExpression*>(expr.get())) {
                std::string name = varExpr->name;
                auto value = parseAssignmentExpression();
                auto assign = std::make_unique<AssignmentExpression>(name, std::move(value));
                assign->line = equals.line;
                assign->column = equals.column;
                return assign;
            } else {
                reportError("Invalid assignment target");
            }
        }

        return expr;
    }

    std::unique_ptr<Expression> Parser::parseEquality() {
        auto expr = parseComparison();

        while (match(TokenType:: EqualEqual) || match(TokenType::BangEqual)) {
            std::string op = previous().value;
            auto right = parseComparison();
            expr = std::make_unique<BinaryExpression>(BinaryExpression{op, std::move(expr), std::move(right)});
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
        if (match(TokenType::Star)) {
            Token className = expect(TokenType::Identifier, "Expected class name after '*'");
            expect(TokenType::LParen, "Expected '(' after class name");
            auto args = parseArgumentList();
            expect(TokenType::RParen, "Expected ')' after arguments");

            return std::make_unique<ConstructorCallExpression>(className.value, std::move(args));
        }

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
            if (match(TokenType::Dot)) {
                expect(TokenType::Identifier, "Expected member name after '.'");
                std::string member = previous().value;
                expr = std::make_unique<MemberAccessExpression>(std::move(expr), member);
            } else if (match(TokenType::LParen)) {
                std::vector<std::unique_ptr<Expression>> args;
                if (!check(TokenType::RParen)) {
                    do {
                        args.push_back(parseExpression());
                    } while (match(TokenType::Comma));
                }
                expect(TokenType::RParen, "Expected ')' after arguments");
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
            expect(TokenType::RParen, "Expected ')' after expression");
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
        // First: handle primitives and logical<>
        if (check(TokenType::Void) || check(TokenType::Logical) || check(TokenType::Int) ||
            check(TokenType::Float) || check(TokenType::Char) || check(TokenType::String) ||
            check(TokenType::Bit) || check(TokenType::Qubit)) {
            auto baseType = parsePrimitiveType();

            // Array types are only allowed for primitive types
            if (match(TokenType::LBracket)) {
                expect(TokenType::RBracket, "Expected ']' after '[' in array type");
                return parseArrayType(std::move(baseType));
            }

            return baseType;
        }

        // Handle ObjectType (class types)
        if (check(TokenType::Identifier)) {
            std::string typeName = advance().value;
            return std::make_unique<ObjectType>(typeName);
        }

        reportError("Expected type");
        return nullptr;
    }

    std::unique_ptr<Type> Parser::parsePrimitiveType() {
        if (check(TokenType::Void)) {
            advance();
            return std::make_unique<VoidType>();
        }

        if (check(TokenType::Logical)) {
            advance();
            expect(TokenType::Less, "Expected '<' after 'logical'");
            if (!check(TokenType::Identifier)) {
                reportError("Expected code identifier inside logical<>");
            }
            std::string code = advance().value;
            expect(TokenType::Greater, "Expected '>' after code identifier");
            return std::make_unique<LogicalType>(code);
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