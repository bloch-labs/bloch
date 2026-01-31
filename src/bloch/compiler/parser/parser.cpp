// Copyright 2025-2026 Akshay Pal (https://bloch-labs.com)
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

#include "bloch/compiler/parser/parser.hpp"

#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>

#include "bloch/support/error/bloch_error.hpp"

namespace bloch::compiler {

    using support::BlochError;
    using support::ErrorCategory;
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
        if (!checkNext(TokenType::Quantum) && !checkNext(TokenType::Shots))
            return false;
        return true;
    }

    bool Parser::isTypeAhead() const {
        if (check(TokenType::Void) || check(TokenType::Int) || check(TokenType::Float) ||
            check(TokenType::Long) || check(TokenType::Char) || check(TokenType::String) ||
            check(TokenType::Bit) || check(TokenType::Qubit) || check(TokenType::Boolean)) {
            return true;
        }
        if (!check(TokenType::Identifier))
            return false;
        size_t idx = m_current;  // start at identifier
        // Consume dotted qualifiers
        while (idx + 2 < m_tokens.size() && m_tokens[idx + 1].type == TokenType::Dot &&
               m_tokens[idx + 2].type == TokenType::Identifier) {
            idx += 2;
        }
        // Skip generic type arguments if present: < ... >
        auto skipTypeArgs = [&](size_t& i) {
            if (i + 1 >= m_tokens.size() || m_tokens[i + 1].type != TokenType::Less)
                return;
            int depth = 0;
            size_t j = i + 1;
            while (j < m_tokens.size()) {
                if (m_tokens[j].type == TokenType::Less)
                    depth++;
                else if (m_tokens[j].type == TokenType::Greater) {
                    depth--;
                    if (depth == 0) {
                        i = j;
                        break;
                    }
                }
                j++;
            }
        };
        skipTypeArgs(idx);
        if (idx + 1 >= m_tokens.size())
            return false;
        TokenType afterName = m_tokens[idx + 1].type;
        if (afterName == TokenType::Identifier) {
            return true;  // Type Name varName
        }
        if (afterName == TokenType::LBracket) {
            size_t j = idx + 2;
            while (j < m_tokens.size() && m_tokens[j].type != TokenType::RBracket) {
                // Bail out on statement terminators to avoid misclassifying expressions
                if (m_tokens[j].type == TokenType::Semicolon || m_tokens[j].type == TokenType::Eof)
                    return false;
                ++j;
            }
            if (j >= m_tokens.size() || m_tokens[j].type != TokenType::RBracket)
                return false;
            if (j + 1 < m_tokens.size() && m_tokens[j + 1].type == TokenType::Identifier) {
                return true;  // Type[] varName
            }
        }
        return false;
    }

    bool Parser::isAtEnd() const { return peek().type == TokenType::Eof; }

    // Error
    void Parser::reportError(const std::string& msg) {
        const Token& token = peek();
        throw BlochError(ErrorCategory::Parse, token.line, token.column, msg);
    }

    // Main parse function
    std::unique_ptr<Program> Parser::parse() {
        std::unique_ptr<Program> program = std::make_unique<Program>();
        // We alternate between function declarations and top-level statements.
        // Extra statements generated from multi-declarations are flushed as we go.
        while (!isAtEnd()) {
            if (match(TokenType::Import)) {
                program->imports.push_back(parseImport());
            } else if (check(TokenType::Static) || check(TokenType::Abstract) ||
                       check(TokenType::Class)) {
                program->classes.push_back(parseClassDeclaration());
            } else if (check(TokenType::Function) || checkFunctionAnnotation()) {
                program->functions.push_back(parseFunction());
            } else {
                program->statements.push_back(parseStatement());
                flushExtraStatements(program->statements);
            }
        }

        return program;
    }

    // Top level

    std::unique_ptr<ImportDeclaration> Parser::parseImport() {
        std::unique_ptr<ImportDeclaration> import = std::make_unique<ImportDeclaration>();
        const Token& importTok = previous();
        import->line = importTok.line;
        import->column = importTok.column;
        import->path = parseQualifiedName();
        (void)expect(TokenType::Semicolon, "Expected ';' after import statement");
        return import;
    }

    std::unique_ptr<ClassDeclaration> Parser::parseClassDeclaration() {
        bool isStatic = false;
        bool isAbstract = false;
        while (true) {
            if (match(TokenType::Static)) {
                if (isStatic)
                    reportError("duplicate 'static' modifier on class");
                isStatic = true;
                continue;
            }
            if (match(TokenType::Abstract)) {
                if (isAbstract)
                    reportError("duplicate 'abstract' modifier on class");
                isAbstract = true;
                continue;
            }
            break;
        }

        (void)expect(TokenType::Class, "Expected 'class' keyword");

        const Token& nameTok = expect(TokenType::Identifier, "Expected class name after 'class'");

        std::unique_ptr<ClassDeclaration> cls = std::make_unique<ClassDeclaration>();
        cls->name = nameTok.value;
        cls->line = nameTok.line;
        cls->column = nameTok.column;
        cls->isStatic = isStatic;
        cls->isAbstract = isAbstract;

        if (check(TokenType::Less)) {
            cls->typeParameters = parseTypeParameters();
        }

        if (match(TokenType::Extends)) {
            auto baseTy = parseType();
            if (!baseTy)
                reportError("Expected base class name after 'extends'");
            if (!dynamic_cast<NamedType*>(baseTy.get())) {
                reportError("Base class must be a named type");
            }
            cls->baseType = std::move(baseTy);
            if (auto named = dynamic_cast<NamedType*>(cls->baseType.get()))
                cls->baseName = named->nameParts;
            if (check(TokenType::Extends)) {
                reportError("Only single inheritance is supported");
            }
        }

        (void)expect(TokenType::LBrace, "Expected '{' to start class body");
        while (!check(TokenType::RBrace) && !isAtEnd()) {
            std::unique_ptr<ClassMember> member = parseClassMember(cls->name, cls->isStatic);
            if (member)
                cls->members.push_back(std::move(member));
        }
        (void)expect(TokenType::RBrace, "Expected '}' to end class body");

        return cls;
    }

    // Function Declaration
    std::unique_ptr<FunctionDeclaration> Parser::parseFunction() {
        std::unique_ptr<FunctionDeclaration> func = std::make_unique<FunctionDeclaration>();

        // Parse annotations
        while (check(TokenType::At)) {
            (void)previous();
            std::unique_ptr<AnnotationNode> annotation = parseFunctionAnnotation();
            // TODO: Refactor this to a switch statement
            if (annotation->name == "quantum") {
                func->hasQuantumAnnotation = true;
            } else if (annotation->name == "shots") {
                func->hasShotsAnnotation = true;
            } else {
                reportError("Invalid annotation name");
            }
            func->annotations.push_back(std::move(annotation));
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
            std::unique_ptr<Parameter> param = std::make_unique<Parameter>();

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

    Visibility Parser::parseVisibility() {
        if (match(TokenType::Public))
            return Visibility::Public;
        if (match(TokenType::Private))
            return Visibility::Private;
        if (match(TokenType::Protected))
            return Visibility::Protected;
        return Visibility::Public;
    }

    std::vector<std::string> Parser::parseQualifiedName() {
        std::vector<std::string> parts;
        const Token& first = expect(TokenType::Identifier, "Expected identifier");
        parts.push_back(first.value);
        while (match(TokenType::Dot)) {
            const Token& part = expect(TokenType::Identifier, "Expected identifier after '.'");
            parts.push_back(part.value);
        }
        return parts;
    }

    std::unique_ptr<ClassMember> Parser::parseClassMember(const std::string& className,
                                                          bool isStaticClass) {
        std::vector<std::unique_ptr<AnnotationNode>> annotations = parseAnnotations();

        bool hasVisibility =
            check(TokenType::Public) || check(TokenType::Private) || check(TokenType::Protected);
        Visibility visibility = hasVisibility
                                    ? parseVisibility()
                                    : (isStaticClass ? Visibility::Public : Visibility::Private);
        if (hasVisibility && (check(TokenType::Public) || check(TokenType::Private) ||
                              check(TokenType::Protected))) {
            reportError("Multiple visibility modifiers are not allowed on class members");
        }

        bool isStatic = false;
        bool isVirtual = false;
        bool isOverride = false;
        bool scanningModifiers = true;
        while (scanningModifiers) {
            if (match(TokenType::Static)) {
                if (isStatic)
                    reportError("Duplicate 'static' modifier");
                isStatic = true;
                continue;
            }
            if (match(TokenType::Virtual)) {
                if (isVirtual)
                    reportError("Duplicate 'virtual' modifier");
                isVirtual = true;
                continue;
            }
            if (match(TokenType::Override)) {
                if (isOverride)
                    reportError("Duplicate 'override' modifier");
                isOverride = true;
                continue;
            }
            scanningModifiers = false;
        }

        std::vector<std::unique_ptr<AnnotationNode>> trailingAnnotations = parseAnnotations();
        for (auto& ann : trailingAnnotations) annotations.push_back(std::move(ann));

        if (match(TokenType::Constructor)) {
            if (!annotations.empty()) {
                reportError("Annotations are not allowed on constructors");
            }
            if (isStaticClass)
                reportError("Static classes cannot declare constructors");
            if (isStatic || isVirtual || isOverride)
                reportError("Constructors cannot be static, virtual, or override");
            return parseConstructorDeclaration(visibility, className);
        }

        if (match(TokenType::Destructor)) {
            if (!annotations.empty()) {
                reportError("Annotations are not allowed on destructors");
            }
            if (isStaticClass)
                reportError("Static classes cannot declare destructors");
            if (isStatic || isVirtual || isOverride)
                reportError("Destructors cannot be static, virtual, or override");
            return parseDestructorDeclaration(visibility);
        }

        if (match(TokenType::Function)) {
            if (isStaticClass && !isStatic) {
                reportError("Static classes may only contain static methods");
            }
            if (isStaticClass && (isVirtual || isOverride)) {
                reportError("Static classes cannot contain virtual or override methods");
            }
            return parseMethodDeclaration(visibility, isStatic, isVirtual, isOverride,
                                          std::move(annotations));
        }

        if (isVirtual || isOverride) {
            reportError("'virtual' or 'override' may only modify methods");
        }
        if (isStaticClass && !isStatic) {
            reportError("Static classes may only contain static members");
        }

        bool isFinalField = match(TokenType::Final);
        return parseFieldDeclaration(visibility, isFinalField, isStatic, std::move(annotations));
    }

    std::unique_ptr<FieldDeclaration> Parser::parseFieldDeclaration(
        Visibility vis, bool isFinal, bool isStatic,
        std::vector<std::unique_ptr<AnnotationNode>> annotations) {
        std::unique_ptr<FieldDeclaration> field = std::make_unique<FieldDeclaration>();
        field->visibility = vis;
        field->isFinal = isFinal;
        field->isStatic = isStatic;

        field->annotations = std::move(annotations);
        for (auto& ann : field->annotations) {
            if (ann && ann->name == "tracked")
                field->isTracked = true;
        }

        field->fieldType = parseType();

        const Token& nameTok = expect(TokenType::Identifier, "Expected field name");
        field->name = nameTok.value;
        field->line = nameTok.line;
        field->column = nameTok.column;

        if (match(TokenType::Equals)) {
            field->initializer = parseExpression();
        }

        (void)expect(TokenType::Semicolon, "Expected ';' after field declaration");
        return field;
    }

    std::unique_ptr<MethodDeclaration> Parser::parseMethodDeclaration(
        Visibility vis, bool isStatic, bool isVirtual, bool isOverride,
        std::vector<std::unique_ptr<AnnotationNode>> annotations) {
        std::unique_ptr<MethodDeclaration> method = std::make_unique<MethodDeclaration>();
        method->visibility = vis;
        method->isStatic = isStatic;
        method->isVirtual = isVirtual;
        method->isOverride = isOverride;
        method->annotations = std::move(annotations);
        for (auto& ann : method->annotations) {
            if (ann && ann->name == "quantum")
                method->hasQuantumAnnotation = true;
        }

        const Token& nameTok = expect(TokenType::Identifier, "Expected method name");
        method->name = nameTok.value;
        method->line = nameTok.line;
        method->column = nameTok.column;

        (void)expect(TokenType::LParen, "Expected '(' after method name");
        method->params = parseParameterList();
        (void)expect(TokenType::RParen, "Expected ')' after parameters");

        (void)expect(TokenType::Arrow, "Expected '->' before return type");
        method->returnType = parseType();

        if (check(TokenType::LBrace)) {
            method->body = parseBlock();
        } else {
            if (!method->isVirtual) {
                reportError("Method must have a body unless it is marked 'virtual'");
            }
            (void)expect(TokenType::Semicolon,
                         "Expected ';' after virtual method declaration without a body");
            method->body = nullptr;
        }

        return method;
    }

    std::unique_ptr<ConstructorDeclaration> Parser::parseConstructorDeclaration(
        Visibility vis, const std::string& className) {
        std::unique_ptr<ConstructorDeclaration> ctor = std::make_unique<ConstructorDeclaration>();
        ctor->visibility = vis;
        const Token& ctorTok = previous();
        ctor->line = ctorTok.line;
        ctor->column = ctorTok.column;

        (void)expect(TokenType::LParen, "Expected '(' after 'constructor'");
        ctor->params = parseParameterList();
        (void)expect(TokenType::RParen, "Expected ')' after constructor parameters");
        (void)expect(TokenType::Arrow, "Expected '->' before constructor return type");
        std::unique_ptr<Type> retType = parseType();

        bool matches = false;
        if (auto named = dynamic_cast<NamedType*>(retType.get())) {
            if (!named->nameParts.empty() && named->nameParts.back() == className) {
                matches = true;
            }
        }
        if (!matches) {
            reportError("Constructor must return '" + className + "'");
        }

        if (match(TokenType::Equals)) {
            const Token& defTok = expect(TokenType::Default, "Expected 'default' after '='");
            if (defTok.value != "default") {
                reportError("Only '= default' is supported for constructors");
            }
            (void)expect(TokenType::Semicolon, "Expected ';' after default constructor");
            ctor->isDefault = true;
            ctor->body = nullptr;
        } else {
            ctor->body = parseBlock();
        }
        return ctor;
    }

    std::unique_ptr<DestructorDeclaration> Parser::parseDestructorDeclaration(Visibility vis) {
        std::unique_ptr<DestructorDeclaration> dtor = std::make_unique<DestructorDeclaration>();
        dtor->visibility = vis;
        const Token& dtorTok = previous();
        dtor->line = dtorTok.line;
        dtor->column = dtorTok.column;

        (void)expect(TokenType::LParen, "Expected '(' after 'destructor'");
        if (!check(TokenType::RParen)) {
            reportError("Destructor cannot have parameters");
        }
        (void)expect(TokenType::RParen, "Expected ')' after 'destructor'");

        (void)expect(TokenType::Arrow, "Expected '->' before destructor return type");
        std::unique_ptr<Type> retType = parseType();
        if (!dynamic_cast<VoidType*>(retType.get())) {
            reportError("Destructor must return 'void'");
        }

        if (match(TokenType::Equals)) {
            const Token& defTok = expect(TokenType::Default, "Expected 'default' after '='");
            if (defTok.value != "default") {
                reportError("Only '= default' is supported for destructors");
            }
            (void)expect(TokenType::Semicolon, "Expected ';' after default destructor");
            dtor->isDefault = true;
            dtor->body = nullptr;
        } else {
            dtor->body = parseBlock();
        }
        return dtor;
    }

    // Declarations
    std::unique_ptr<VariableDeclaration> Parser::parseVariableDeclaration(bool isFinal,
                                                                          bool allowMultiple) {
        return parseVariableDeclaration(nullptr, isFinal, allowMultiple);
    }

    std::unique_ptr<VariableDeclaration> Parser::parseVariableDeclaration(
        std::unique_ptr<Type> preParsedType, bool isFinal, bool allowMultiple) {
        std::unique_ptr<VariableDeclaration> var = std::make_unique<VariableDeclaration>();
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
            std::unique_ptr<VariableDeclaration> extraVar = std::make_unique<VariableDeclaration>();
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

    // @tracked
    std::unique_ptr<AnnotationNode> Parser::parseVariableAnnotation() {
        (void)expect(TokenType::At, "Expected '@' to begin annotation");

        if (!check(TokenType::Tracked)) {
            const Token& invalid = peek();
            std::string invalidName = invalid.value.empty() ? std::string("") : invalid.value;
            reportError(std::string("\"") + "@" + invalidName +
                        "\" is not a valid Bloch variable annotation");
        }
        Token annotationToken = advance();
        std::unique_ptr<AnnotationNode> annotation = std::make_unique<AnnotationNode>();
        annotation->name = annotationToken.value;
        annotation->isVariableAnnotation = true;
        return annotation;
    }

    //@quantum, @shots(N)
    std::unique_ptr<AnnotationNode> Parser::parseFunctionAnnotation() {
        (void)expect(TokenType::At, "Expected '@' to begin annotation");

        if (!check(TokenType::Quantum) && !check(TokenType::Shots)) {
            const Token& invalid = peek();
            std::string invalidName = invalid.value.empty() ? std::string("") : invalid.value;
            reportError(std::string("\"") + "@" + invalidName +
                        "\" is not a valid Bloch function/method annotation");
        }
        Token annotationToken = advance();
        Token numberOfShots;
        if (annotationToken.type == TokenType::Shots) {
            (void)expect(TokenType::LParen, "Expected opening bracket '('");
            numberOfShots = expect(TokenType::IntegerLiteral, "Number of shots must be an integer");
            (void)expect(TokenType::RParen, "Expected closing bracket ')'");
        }
        std::unique_ptr<AnnotationNode> annotation = std::make_unique<AnnotationNode>();
        annotation->name = annotationToken.value;
        annotation->value = numberOfShots.value.empty() ? std::string{""} : numberOfShots.value;
        annotation->isFunctionAnnotation = true;
        return annotation;
    }

    std::vector<std::unique_ptr<AnnotationNode>> Parser::parseAnnotations() {
        std::vector<std::unique_ptr<AnnotationNode>> annotations;

        while (check(TokenType::At)) {
            // TODO: refactor this, currently if invalid variable annotation is used, it will be
            // caught rather than thrown this is a rather hacky solution.
            try {
                annotations.push_back(parseVariableAnnotation());
            } catch (BlochError error) {
                annotations.push_back(parseFunctionAnnotation());
            }
        }

        return annotations;
    }

    // Statements
    std::unique_ptr<Statement> Parser::parseStatement() {
        if (check(TokenType::LBrace))
            return parseBlock();

        bool isFinal = match(TokenType::Final);
        bool typeAhead = isTypeAhead();

        // Match primitive declarations or annotated declarations
        if (check(TokenType::At) || typeAhead) {
            if (isFinal && !typeAhead && !check(TokenType::At)) {
                reportError("Expected variable type after 'final'");
            }
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
        if (match(TokenType::Destroy))
            return parseDestroy();

        if (check(TokenType::Identifier) && checkNext(TokenType::Equals))
            return parseAssignment();

        std::unique_ptr<Expression> expr = parseExpression();
        if (match(TokenType::Question)) {
            std::unique_ptr<Statement> thenBranch = parseStatement();
            (void)expect(TokenType::Colon, "Expected ':' after true branch");
            std::unique_ptr<Statement> elseBranch = parseStatement();
            std::unique_ptr<TernaryStatement> stmt = std::make_unique<TernaryStatement>();
            stmt->condition = std::move(expr);
            stmt->thenBranch = std::move(thenBranch);
            stmt->elseBranch = std::move(elseBranch);
            return stmt;
        }

        (void)expect(TokenType::Semicolon, "Expected ';' after expression");
        std::unique_ptr<ExpressionStatement> stmt = std::make_unique<ExpressionStatement>();
        stmt->expression = std::move(expr);
        return stmt;
    }

    // {...}
    std::unique_ptr<BlockStatement> Parser::parseBlock() {
        const Token& lbrace = expect(TokenType::LBrace, "Expected '{' to start block");

        std::unique_ptr<BlockStatement> block = std::make_unique<BlockStatement>();
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
        std::unique_ptr<ReturnStatement> stmt = std::make_unique<ReturnStatement>();
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
        std::unique_ptr<Expression> condition = parseExpression();
        (void)expect(TokenType::RParen, "Expected ')' after condition");

        std::unique_ptr<BlockStatement> thenBranch = parseBlock();

        std::unique_ptr<BlockStatement> elseBranch = nullptr;
        if (match(TokenType::Else)) {
            elseBranch = parseBlock();
        }

        std::unique_ptr<IfStatement> stmt = std::make_unique<IfStatement>();
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

        std::unique_ptr<Expression> condition = parseExpression();
        (void)expect(TokenType::Semicolon, "Expected ';' after loop condition");

        std::unique_ptr<Expression> increment = parseExpression();
        (void)expect(TokenType::RParen, "Expected ')' after for clause");

        std::unique_ptr<BlockStatement> body = parseBlock();

        std::unique_ptr<ForStatement> stmt = std::make_unique<ForStatement>();
        stmt->initializer = std::move(initializer);
        stmt->condition = std::move(condition);
        stmt->increment = std::move(increment);
        stmt->body = std::move(body);

        return stmt;
    }

    // while (cond) {...}
    std::unique_ptr<WhileStatement> Parser::parseWhile() {
        (void)expect(TokenType::LParen, "Expected '(' after 'while'");
        std::unique_ptr<Expression> condition = parseExpression();
        (void)expect(TokenType::RParen, "Expected ')' after condition");
        std::unique_ptr<BlockStatement> body = parseBlock();

        std::unique_ptr<WhileStatement> stmt = std::make_unique<WhileStatement>();
        stmt->condition = std::move(condition);
        stmt->body = std::move(body);
        return stmt;
    }

    // echo(expr);
    std::unique_ptr<EchoStatement> Parser::parseEcho() {
        const Token& echoTok = previous();
        (void)expect(TokenType::LParen, "Expected '(' after 'echo'");
        std::unique_ptr<Expression> value = parseExpression();
        (void)expect(TokenType::RParen, "Expected ')' after echo argument");
        (void)expect(TokenType::Semicolon, "Expected ';' after echo statement");

        std::unique_ptr<EchoStatement> stmt = std::make_unique<EchoStatement>();
        stmt->value = std::move(value);
        stmt->line = echoTok.line;
        stmt->column = echoTok.column;
        return stmt;
    }

    // reset q0;
    std::unique_ptr<ResetStatement> Parser::parseReset() {
        const Token& resetTok = previous();
        std::unique_ptr<ResetStatement> stmt = std::make_unique<ResetStatement>();
        stmt->target = parseExpression();
        (void)expect(TokenType::Semicolon, "Expected ';' after reset target");
        stmt->line = resetTok.line;
        stmt->column = resetTok.column;
        return stmt;
    }

    // measure q0;
    std::unique_ptr<MeasureStatement> Parser::parseMeasure() {
        const Token& measureTok = previous();
        std::unique_ptr<MeasureStatement> stmt = std::make_unique<MeasureStatement>();
        stmt->qubit = parseExpression();
        (void)expect(TokenType::Semicolon, "Expected ';' after measure target");
        stmt->line = measureTok.line;
        stmt->column = measureTok.column;
        return stmt;
    }

    // destroy expr;
    std::unique_ptr<DestroyStatement> Parser::parseDestroy() {
        const Token& destroyTok = previous();
        std::unique_ptr<DestroyStatement> stmt = std::make_unique<DestroyStatement>();
        stmt->target = parseExpression();
        (void)expect(TokenType::Semicolon, "Expected ';' after destroy target");
        stmt->line = destroyTok.line;
        stmt->column = destroyTok.column;
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

        std::unique_ptr<AssignmentStatement> stmt = std::make_unique<AssignmentStatement>();
        stmt->name = name;
        stmt->line = nameToken.line;
        stmt->column = nameToken.column;
        stmt->value = parseExpression();

        (void)expect(TokenType::Semicolon, "Expected ';' after assignment");
        return stmt;
    }

    std::unique_ptr<ExpressionStatement> Parser::parseExpressionStatement() {
        std::unique_ptr<Expression> expr = parseExpression();
        (void)expect(TokenType::Semicolon, "Expected ';' after expression");
        std::unique_ptr<ExpressionStatement> stmt = std::make_unique<ExpressionStatement>();
        stmt->expression = std::move(expr);
        return stmt;
    }

    // Expressions (Pratt / precedence-table based)
    std::unique_ptr<Expression> Parser::parseExpression() { return parseAssignmentExpression(); }

    namespace {
        /**
         * Binding powers for Pratt parsing.
         * lbp = left binding power, rbp = right binding power.
         * For left-associative infix operators: lbp == rbp - 1.
         * For postfix operators we reuse the table with Kind::Postfix.
         * New operators are added by extending infixBinding() below.
         */
        struct Binding {
            int lbp;
            int rbp;
            enum class Kind { Infix, Postfix } kind = Kind::Infix;
        };

        // Returns binding power for an operator if it is handled in the Pratt loop.
        std::optional<Binding> infixBinding(TokenType type) {
            switch (type) {
                case TokenType::PipePipe:
                    return Binding{3, 4, Binding::Kind::Infix};  // logical or (left)
                case TokenType::AmpersandAmpersand:
                    return Binding{4, 5, Binding::Kind::Infix};  // logical and (left)
                case TokenType::Pipe:
                    return Binding{5, 6, Binding::Kind::Infix};  // bitwise or (left)
                case TokenType::Caret:
                    return Binding{6, 7, Binding::Kind::Infix};  // bitwise xor (left)
                case TokenType::Ampersand:
                    return Binding{7, 8, Binding::Kind::Infix};  // bitwise and (left)
                case TokenType::EqualEqual:
                case TokenType::BangEqual:
                    return Binding{8, 9, Binding::Kind::Infix};  // equality (left)
                case TokenType::Greater:
                case TokenType::Less:
                case TokenType::GreaterEqual:
                case TokenType::LessEqual:
                    return Binding{9, 10, Binding::Kind::Infix};  // relational (left)
                case TokenType::Plus:
                case TokenType::Minus:
                    return Binding{11, 12, Binding::Kind::Infix};  // additive (left)
                case TokenType::Star:
                case TokenType::Slash:
                case TokenType::Percent:
                    return Binding{12, 13, Binding::Kind::Infix};  // multiplicative (left)
                case TokenType::Dot:
                case TokenType::LParen:    // call
                case TokenType::LBracket:  // index
                case TokenType::PlusPlus:  // postfix
                case TokenType::MinusMinus:
                    return Binding{16, 17, Binding::Kind::Postfix};  // postfix/call/member
                default:
                    return std::nullopt;
            }
        }

        constexpr int kPrefixBindingPower =
            14;  // binds tighter than binary ops, looser than postfix
    }            // namespace

    std::unique_ptr<Expression> Parser::parseAssignmentExpression() {
        // Right-associative assignment built on top of Pratt for the rest.
        std::unique_ptr<Expression> left = parsePrattExpression(0);

        if (match(TokenType::Equals)) {
            std::unique_ptr<Expression> value = parseAssignmentExpression();

            if (auto varExpr = dynamic_cast<VariableExpression*>(left.get())) {
                int line = varExpr->line;
                int column = varExpr->column;
                std::string name = varExpr->name;
                std::unique_ptr<AssignmentExpression> assign =
                    std::make_unique<AssignmentExpression>(name, std::move(value));
                assign->line = line;
                assign->column = column;
                return assign;
            } else if (dynamic_cast<IndexExpression*>(left.get())) {
                std::unique_ptr<IndexExpression> idx(static_cast<IndexExpression*>(left.release()));
                int line = idx->line;
                int column = idx->column;
                std::unique_ptr<ArrayAssignmentExpression> arrayAssign =
                    std::make_unique<ArrayAssignmentExpression>(
                        std::move(idx->collection), std::move(idx->index), std::move(value));
                arrayAssign->line = line;
                arrayAssign->column = column;
                return arrayAssign;
            } else if (dynamic_cast<MemberAccessExpression*>(left.get())) {
                std::unique_ptr<MemberAccessExpression> mem(
                    static_cast<MemberAccessExpression*>(left.release()));
                int line = mem->line;
                int column = mem->column;
                std::unique_ptr<MemberAssignmentExpression> memberAssign =
                    std::make_unique<MemberAssignmentExpression>(std::move(mem->object),
                                                                 mem->member, std::move(value));
                memberAssign->line = line;
                memberAssign->column = column;
                return memberAssign;
            }

            reportError("Invalid assignment target");
        }

        return left;
    }

    std::unique_ptr<Expression> Parser::parsePrattExpression(int minBp) {
        std::unique_ptr<Expression> left = parsePrefixExpression();

        while (true) {
            const Token& tok = peek();
            auto binding = infixBinding(tok.type);
            if (!binding || binding->lbp < minBp)
                break;

            (void)advance();  // consume operator / postfix marker

            if (binding->kind == Binding::Kind::Postfix) {
                switch (tok.type) {
                    case TokenType::LParen: {
                        const Token& lparen = tok;
                        std::vector<std::unique_ptr<Expression>> args;
                        if (!check(TokenType::RParen)) {
                            do {
                                args.push_back(parseExpression());
                            } while (match(TokenType::Comma));
                        }
                        (void)expect(TokenType::RParen, "Expected ')' after arguments");
                        int calleeLine = left ? left->line : lparen.line;
                        int calleeColumn = left ? left->column : lparen.column;
                        std::unique_ptr<CallExpression> call =
                            std::make_unique<CallExpression>(std::move(left), std::move(args));
                        call->line = calleeLine;
                        call->column = calleeColumn;
                        left = std::move(call);
                        continue;
                    }
                    case TokenType::LBracket: {
                        const Token& lbr = tok;
                        std::unique_ptr<IndexExpression> idxExpr =
                            std::make_unique<IndexExpression>();
                        idxExpr->collection = std::move(left);
                        std::unique_ptr<Expression> indexNode = parseExpression();

                        // Constant negative index guard preserved from legacy parser.
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
                                if (auto rlit =
                                        dynamic_cast<LiteralExpression*>(unary->right.get())) {
                                    if (rlit->literalType == "int") {
                                        try {
                                            int v = std::stoi(rlit->value);
                                            negativeConst = v > 0;
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
                        left = std::move(idxExpr);
                        continue;
                    }
                    case TokenType::Dot: {
                        const Token& dotTok = tok;
                        const Token& memberTok =
                            expect(TokenType::Identifier, "Expected member name after '.'");
                        std::unique_ptr<MemberAccessExpression> member =
                            std::make_unique<MemberAccessExpression>();
                        member->object = std::move(left);
                        member->member = memberTok.value;
                        member->line = dotTok.line;
                        member->column = dotTok.column;
                        left = std::move(member);
                        continue;
                    }
                    case TokenType::PlusPlus:
                    case TokenType::MinusMinus: {
                        std::unique_ptr<PostfixExpression> post =
                            std::make_unique<PostfixExpression>(tok.value, std::move(left));
                        post->line = tok.line;
                        post->column = tok.column;
                        left = std::move(post);
                        continue;
                    }
                    default:
                        break;
                }
            }

            // Infix binary
            std::unique_ptr<Expression> right = parsePrattExpression(binding->rbp);
            std::unique_ptr<BinaryExpression> bin =
                std::make_unique<BinaryExpression>(tok.value, std::move(left), std::move(right));
            bin->line = tok.line;
            bin->column = tok.column;
            left = std::move(bin);
        }

        return left;
    }

    std::unique_ptr<Expression> Parser::parsePrefixExpression() {
        const Token& tok = peek();
        if (tok.type == TokenType::Minus || tok.type == TokenType::Bang ||
            tok.type == TokenType::Tilde) {
            (void)advance();
            std::unique_ptr<Expression> right = parsePrattExpression(kPrefixBindingPower);
            std::unique_ptr<UnaryExpression> un =
                std::make_unique<UnaryExpression>(tok.value, std::move(right));
            un->line = tok.line;
            un->column = tok.column;
            return un;
        }

        return parsePrimary();
    }

    std::unique_ptr<Expression> Parser::parseUnary() { return parsePrefixExpression(); }

    std::unique_ptr<Expression> Parser::parsePrimary() {
        if (match(TokenType::IntegerLiteral) || match(TokenType::LongLiteral) ||
            match(TokenType::FloatLiteral) || match(TokenType::BitLiteral) ||
            match(TokenType::StringLiteral) || match(TokenType::CharLiteral) ||
            match(TokenType::True) || match(TokenType::False)) {
            Token tok = previous();
            std::string litType;
            switch (tok.type) {
                case TokenType::IntegerLiteral:
                    litType = "int";
                    break;
                case TokenType::LongLiteral:
                    litType = "long";
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
                case TokenType::True:
                case TokenType::False:
                    litType = "boolean";
                    break;
                case TokenType::StringLiteral:
                    litType = "string";
                    break;
                default:
                    break;
            }
            return std::make_unique<LiteralExpression>(LiteralExpression{tok.value, litType});
        }

        if (match(TokenType::Null)) {
            const Token& tok = previous();
            std::unique_ptr<NullLiteralExpression> expr = std::make_unique<NullLiteralExpression>();
            expr->line = tok.line;
            expr->column = tok.column;
            return expr;
        }

        if (match(TokenType::Measure)) {
            const Token& measureTok = previous();
            std::unique_ptr<Expression> target = parseExpression();
            std::unique_ptr<MeasureExpression> expr =
                std::make_unique<MeasureExpression>(MeasureExpression{std::move(target)});
            expr->line = measureTok.line;
            expr->column = measureTok.column;
            return expr;
        }

        if (match(TokenType::This)) {
            const Token& tok = previous();
            std::unique_ptr<ThisExpression> expr = std::make_unique<ThisExpression>();
            expr->line = tok.line;
            expr->column = tok.column;
            return expr;
        }

        if (match(TokenType::Super)) {
            const Token& tok = previous();
            std::unique_ptr<SuperExpression> expr = std::make_unique<SuperExpression>();
            expr->line = tok.line;
            expr->column = tok.column;
            return expr;
        }

        if (match(TokenType::New)) {
            const Token& newTok = previous();
            std::unique_ptr<Type> type = parseType();
            (void)expect(TokenType::LParen, "Expected '(' after type in 'new' expression");
            std::vector<std::unique_ptr<Expression>> args = parseArgumentList();
            (void)expect(TokenType::RParen, "Expected ')' after arguments");
            std::unique_ptr<NewExpression> expr = std::make_unique<NewExpression>();
            expr->classType = std::move(type);
            expr->arguments = std::move(args);
            expr->line = newTok.line;
            expr->column = newTok.column;
            return expr;
        }

        if (match(TokenType::Identifier)) {
            const Token& token = previous();
            std::unique_ptr<VariableExpression> expr =
                std::make_unique<VariableExpression>(VariableExpression{token.value});
            expr->line = token.line;
            expr->column = token.column;
            return expr;
        }

        if (match(TokenType::LBrace)) {
            Token start = previous();
            std::unique_ptr<Expression> expr = parseArrayLiteral();
            expr->line = start.line;
            expr->column = start.column;
            return expr;
        }

        if (match(TokenType::LParen)) {
            const Token& lparen = previous();
            if (isTypeAhead()) {
                std::unique_ptr<Type> targetType = parseType();
                (void)expect(TokenType::RParen, "Expected ')' after type in cast expression");
                std::unique_ptr<Expression> operand = parseUnary();
                std::unique_ptr<CastExpression> cast =
                    std::make_unique<CastExpression>(std::move(targetType), std::move(operand));
                cast->line = lparen.line;
                cast->column = lparen.column;
                return cast;
            }
            std::unique_ptr<Expression> expr = parseExpression();
            (void)expect(TokenType::RParen, "Expected ')' after expression");
            auto paren =
                std::make_unique<ParenthesizedExpression>(ParenthesizedExpression{std::move(expr)});
            paren->line = lparen.line;
            paren->column = lparen.column;
            return paren;
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
            case TokenType::LongLiteral:
                return std::make_unique<LiteralExpression>(LiteralExpression{token.value, "long"});
            case TokenType::FloatLiteral:
                return std::make_unique<LiteralExpression>(LiteralExpression{token.value, "float"});
            case TokenType::BitLiteral:
                return std::make_unique<LiteralExpression>(LiteralExpression{token.value, "bit"});
            case TokenType::CharLiteral:
                return std::make_unique<LiteralExpression>(LiteralExpression{token.value, "char"});
            case TokenType::StringLiteral:
                return std::make_unique<LiteralExpression>(
                    LiteralExpression{token.value, "string"});
            case TokenType::Null: {
                auto expr = std::make_unique<NullLiteralExpression>();
                expr->line = token.line;
                expr->column = token.column;
                return expr;
            }
            default:
                reportError("Expected a literal value.");
                return nullptr;
        }
    }

    // Types

    std::unique_ptr<Type> Parser::parseType() {
        std::unique_ptr<Type> baseType;
        if (check(TokenType::Void)) {
            (void)advance();
            baseType = std::make_unique<VoidType>();
        } else if (check(TokenType::Int) || check(TokenType::Long) || check(TokenType::Float) ||
                   check(TokenType::Char) || check(TokenType::String) || check(TokenType::Bit) ||
                   check(TokenType::Qubit) || check(TokenType::Boolean)) {
            baseType = parsePrimitiveType();
        } else if (check(TokenType::Identifier)) {
            std::vector<std::string> parts = parseQualifiedName();
            auto named = std::make_unique<NamedType>(parts);
            if (match(TokenType::Less)) {
                named->typeArguments = parseTypeArgumentList();
            }
            baseType = std::move(named);
        } else {
            reportError("Expected type");
            return nullptr;
        }

        while (match(TokenType::LBracket)) {
            int arrSize = -1;
            std::unique_ptr<Expression> sizeExpr = nullptr;
            if (!check(TokenType::RBracket)) {
                if (check(TokenType::IntegerLiteral)) {
                    const Token& sizeTok = advance();
                    try {
                        arrSize = std::stoi(sizeTok.value);
                    } catch (...) {
                        reportError("Invalid integer size in array type");
                    }
                } else {
                    sizeExpr = parseExpression();
                }
            }
            (void)expect(TokenType::RBracket, "Expected ']' after '[' in array type");
            if (dynamic_cast<VoidType*>(baseType.get())) {
                reportError("array element type cannot be 'void'");
            }
            baseType = parseArrayType(std::move(baseType), arrSize, std::move(sizeExpr));
        }

        return baseType;
    }

    std::unique_ptr<Type> Parser::parsePrimitiveType() {
        if (check(TokenType::Int) || check(TokenType::Long) || check(TokenType::Float) ||
            check(TokenType::Char) || check(TokenType::String) || check(TokenType::Bit) ||
            check(TokenType::Qubit) || check(TokenType::Boolean)) {
            std::string typeName = advance().value;
            return std::make_unique<PrimitiveType>(typeName);
        }

        reportError("Expected primitive type");
        return nullptr;
    }

    std::unique_ptr<Type> Parser::parseArrayType(std::unique_ptr<Type> elementType, int size,
                                                 std::unique_ptr<Expression> sizeExpr) {
        return std::make_unique<ArrayType>(std::move(elementType), size, std::move(sizeExpr));
    }

    std::vector<std::unique_ptr<TypeParameter>> Parser::parseTypeParameters() {
        std::vector<std::unique_ptr<TypeParameter>> params;
        (void)expect(TokenType::Less, "Expected '<' to start type parameters");
        if (check(TokenType::Greater)) {
            (void)advance();
            return params;
        }
        do {
            const Token& nameTok = expect(TokenType::Identifier, "Expected type parameter name");
            auto param = std::make_unique<TypeParameter>();
            param->name = nameTok.value;
            param->line = nameTok.line;
            param->column = nameTok.column;
            if (match(TokenType::Extends)) {
                param->bound = parseType();
            }
            params.push_back(std::move(param));
        } while (match(TokenType::Comma));
        (void)expect(TokenType::Greater, "Expected '>' to end type parameters");
        return params;
    }

    std::vector<std::unique_ptr<Type>> Parser::parseTypeArgumentList() {
        std::vector<std::unique_ptr<Type>> args;
        do {
            args.push_back(parseType());
        } while (match(TokenType::Comma));
        (void)expect(TokenType::Greater, "Expected '>' after type arguments");
        return args;
    }

    // Parameters and Argments
    std::vector<std::unique_ptr<Parameter>> Parser::parseParameterList() {
        std::vector<std::unique_ptr<Parameter>> parameters;

        while (!check(TokenType::RParen)) {
            std::unique_ptr<Parameter> param = std::make_unique<Parameter>();

            // Parse type
            param->type = parseType();

            // Parse name
            if (!check(TokenType::Identifier)) {
                reportError("Expected parameter name.");
            }
            const Token& paramTok = advance();
            param->name = paramTok.value;
            param->line = paramTok.line;
            param->column = paramTok.column;

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

    std::unique_ptr<Expression> Parser::cloneExpression(const Expression& expr) {
        if (auto lit = dynamic_cast<const LiteralExpression*>(&expr)) {
            std::unique_ptr<LiteralExpression> clone =
                std::make_unique<LiteralExpression>(lit->value, lit->literalType);
            clone->line = lit->line;
            clone->column = lit->column;
            return clone;
        }
        if (auto nullLit = dynamic_cast<const NullLiteralExpression*>(&expr)) {
            std::unique_ptr<NullLiteralExpression> clone =
                std::make_unique<NullLiteralExpression>();
            clone->line = nullLit->line;
            clone->column = nullLit->column;
            return clone;
        }
        if (auto var = dynamic_cast<const VariableExpression*>(&expr)) {
            std::unique_ptr<VariableExpression> clone =
                std::make_unique<VariableExpression>(var->name);
            clone->line = var->line;
            clone->column = var->column;
            return clone;
        }
        if (auto bin = dynamic_cast<const BinaryExpression*>(&expr)) {
            std::unique_ptr<Expression> left = cloneExpression(*bin->left);
            std::unique_ptr<Expression> right = cloneExpression(*bin->right);
            std::unique_ptr<BinaryExpression> clone =
                std::make_unique<BinaryExpression>(bin->op, std::move(left), std::move(right));
            clone->line = bin->line;
            clone->column = bin->column;
            return clone;
        }
        if (auto un = dynamic_cast<const UnaryExpression*>(&expr)) {
            std::unique_ptr<Expression> right = cloneExpression(*un->right);
            std::unique_ptr<UnaryExpression> clone =
                std::make_unique<UnaryExpression>(un->op, std::move(right));
            clone->line = un->line;
            clone->column = un->column;
            return clone;
        }
        if (auto cast = dynamic_cast<const CastExpression*>(&expr)) {
            std::unique_ptr<Type> target;
            if (cast->targetType)
                target = cloneType(*cast->targetType);
            std::unique_ptr<Expression> inner = cloneExpression(*cast->expression);
            std::unique_ptr<CastExpression> clone =
                std::make_unique<CastExpression>(std::move(target), std::move(inner));
            clone->line = cast->line;
            clone->column = cast->column;
            return clone;
        }
        if (auto post = dynamic_cast<const PostfixExpression*>(&expr)) {
            std::unique_ptr<Expression> left = cloneExpression(*post->left);
            std::unique_ptr<PostfixExpression> clone =
                std::make_unique<PostfixExpression>(post->op, std::move(left));
            clone->line = post->line;
            clone->column = post->column;
            return clone;
        }
        if (auto call = dynamic_cast<const CallExpression*>(&expr)) {
            std::unique_ptr<Expression> callee = cloneExpression(*call->callee);
            std::vector<std::unique_ptr<Expression>> args;
            for (const auto& arg : call->arguments) {
                args.push_back(cloneExpression(*arg));
            }
            std::unique_ptr<CallExpression> clone =
                std::make_unique<CallExpression>(std::move(callee), std::move(args));
            clone->line = call->line;
            clone->column = call->column;
            return clone;
        }
        if (auto member = dynamic_cast<const MemberAccessExpression*>(&expr)) {
            std::unique_ptr<Expression> object = cloneExpression(*member->object);
            std::unique_ptr<MemberAccessExpression> clone =
                std::make_unique<MemberAccessExpression>();
            clone->object = std::move(object);
            clone->member = member->member;
            clone->line = member->line;
            clone->column = member->column;
            return clone;
        }
        if (auto newExpr = dynamic_cast<const NewExpression*>(&expr)) {
            std::unique_ptr<Type> classType;
            if (newExpr->classType)
                classType = cloneType(*newExpr->classType);
            std::vector<std::unique_ptr<Expression>> args;
            for (const auto& arg : newExpr->arguments) {
                args.push_back(cloneExpression(*arg));
            }
            std::unique_ptr<NewExpression> clone = std::make_unique<NewExpression>();
            clone->classType = std::move(classType);
            clone->arguments = std::move(args);
            clone->line = newExpr->line;
            clone->column = newExpr->column;
            return clone;
        }
        if (auto thisExpr = dynamic_cast<const ThisExpression*>(&expr)) {
            std::unique_ptr<ThisExpression> clone = std::make_unique<ThisExpression>();
            clone->line = thisExpr->line;
            clone->column = thisExpr->column;
            return clone;
        }
        if (auto superExpr = dynamic_cast<const SuperExpression*>(&expr)) {
            std::unique_ptr<SuperExpression> clone = std::make_unique<SuperExpression>();
            clone->line = superExpr->line;
            clone->column = superExpr->column;
            return clone;
        }
        if (auto idx = dynamic_cast<const IndexExpression*>(&expr)) {
            std::unique_ptr<Expression> collection = cloneExpression(*idx->collection);
            std::unique_ptr<Expression> index = cloneExpression(*idx->index);
            std::unique_ptr<IndexExpression> clone = std::make_unique<IndexExpression>();
            clone->collection = std::move(collection);
            clone->index = std::move(index);
            clone->line = idx->line;
            clone->column = idx->column;
            return clone;
        }
        if (auto arr = dynamic_cast<const ArrayLiteralExpression*>(&expr)) {
            std::vector<std::unique_ptr<Expression>> elems;
            for (const auto& el : arr->elements) {
                elems.push_back(cloneExpression(*el));
            }
            std::unique_ptr<ArrayLiteralExpression> clone =
                std::make_unique<ArrayLiteralExpression>(std::move(elems));
            clone->line = arr->line;
            clone->column = arr->column;
            return clone;
        }
        if (auto par = dynamic_cast<const ParenthesizedExpression*>(&expr)) {
            std::unique_ptr<Expression> inner = cloneExpression(*par->expression);
            std::unique_ptr<ParenthesizedExpression> clone =
                std::make_unique<ParenthesizedExpression>(std::move(inner));
            clone->line = par->line;
            clone->column = par->column;
            return clone;
        }
        if (auto meas = dynamic_cast<const MeasureExpression*>(&expr)) {
            std::unique_ptr<Expression> target = cloneExpression(*meas->qubit);
            std::unique_ptr<MeasureExpression> clone =
                std::make_unique<MeasureExpression>(std::move(target));
            clone->line = meas->line;
            clone->column = meas->column;
            return clone;
        }
        if (auto assign = dynamic_cast<const AssignmentExpression*>(&expr)) {
            std::unique_ptr<Expression> value = cloneExpression(*assign->value);
            std::unique_ptr<AssignmentExpression> clone = std::make_unique<AssignmentExpression>(
                AssignmentExpression{assign->name, std::move(value)});
            clone->line = assign->line;
            clone->column = assign->column;
            return clone;
        }
        if (auto memberAssign = dynamic_cast<const MemberAssignmentExpression*>(&expr)) {
            std::unique_ptr<Expression> object = cloneExpression(*memberAssign->object);
            std::unique_ptr<Expression> value = cloneExpression(*memberAssign->value);
            std::unique_ptr<MemberAssignmentExpression> clone =
                std::make_unique<MemberAssignmentExpression>(
                    std::move(object), memberAssign->member, std::move(value));
            clone->line = memberAssign->line;
            clone->column = memberAssign->column;
            return clone;
        }
        if (auto arrAssign = dynamic_cast<const ArrayAssignmentExpression*>(&expr)) {
            std::unique_ptr<Expression> collection = cloneExpression(*arrAssign->collection);
            std::unique_ptr<Expression> index = cloneExpression(*arrAssign->index);
            std::unique_ptr<Expression> value = cloneExpression(*arrAssign->value);
            std::unique_ptr<ArrayAssignmentExpression> clone =
                std::make_unique<ArrayAssignmentExpression>(std::move(collection), std::move(index),
                                                            std::move(value));
            clone->line = arrAssign->line;
            clone->column = arrAssign->column;
            return clone;
        }
        return nullptr;
    }

    std::unique_ptr<Type> Parser::cloneType(const Type& type) {
        if (auto prim = dynamic_cast<const PrimitiveType*>(&type))
            return std::make_unique<PrimitiveType>(prim->name);
        if (auto named = dynamic_cast<const NamedType*>(&type)) {
            std::unique_ptr<NamedType> clone = std::make_unique<NamedType>(named->nameParts);
            for (const auto& arg : named->typeArguments) {
                clone->typeArguments.push_back(cloneType(*arg));
            }
            clone->line = named->line;
            clone->column = named->column;
            return clone;
        }
        if (auto array = dynamic_cast<const ArrayType*>(&type)) {
            std::unique_ptr<Expression> sizeExprClone;
            if (array->sizeExpression)
                sizeExprClone = cloneExpression(*array->sizeExpression);
            std::unique_ptr<ArrayType> clone = std::make_unique<ArrayType>(
                cloneType(*array->elementType), array->size, std::move(sizeExprClone));
            clone->line = array->line;
            clone->column = array->column;
            return clone;
        }
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
}  // namespace bloch::compiler
