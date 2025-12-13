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

#include "bloch/core/semantics/semantic_analyser.hpp"

#include <iostream>

#include "bloch/core/semantics/built_ins.hpp"

namespace bloch::core {

    using support::BlochError;
    using support::ErrorCategory;

    void SemanticAnalyser::analyse(Program& program) {
        // Global scope for top-level declarations and functions.
        beginScope();
        program.accept(*this);
        endScope();
    }

    ValueType SemanticAnalyser::inferType(Expression* expr) const {
        if (!expr)
            return ValueType::Unknown;
        if (auto lit = dynamic_cast<LiteralExpression*>(expr)) {
            return typeFromString(lit->literalType);
        }
        if (auto var = dynamic_cast<VariableExpression*>(expr)) {
            return getVariableType(var->name);
        }
        if (auto par = dynamic_cast<ParenthesizedExpression*>(expr)) {
            return inferType(par->expression.get());
        }
        if (dynamic_cast<MeasureExpression*>(expr)) {
            return ValueType::Bit;
        }
        if (auto call = dynamic_cast<CallExpression*>(expr)) {
            if (auto callee = dynamic_cast<VariableExpression*>(call->callee.get())) {
                auto it = m_functionInfo.find(callee->name);
                if (it != m_functionInfo.end())
                    return it->second.returnType;
                auto bi = builtInGates.find(callee->name);
                if (bi != builtInGates.end())
                    return bi->second.returnType;
            }
            return ValueType::Unknown;
        }
        if (auto bin = dynamic_cast<BinaryExpression*>(expr)) {
            // Comparison and logical ops yield bit
            if (bin->op == "==" || bin->op == "!=" || bin->op == "<" || bin->op == ">" ||
                bin->op == "<=" || bin->op == ">=" || bin->op == "&&" || bin->op == "||")
                return ValueType::Bit;
            auto lt = inferType(bin->left.get());
            auto rt = inferType(bin->right.get());
            // String concatenation
            if (bin->op == "+" && (lt == ValueType::String || rt == ValueType::String))
                return ValueType::String;
            if (bin->op == "+" || bin->op == "-" || bin->op == "*" || bin->op == "/" ||
                bin->op == "%") {
                if (lt == ValueType::Float || rt == ValueType::Float)
                    return ValueType::Float;
                return ValueType::Int;
            }
            if (bin->op == "&" || bin->op == "|" || bin->op == "^") {
                if (lt == ValueType::Bit && rt == ValueType::Bit)
                    return ValueType::Bit;
                return ValueType::Unknown;
            }
            return ValueType::Unknown;
        }
        if (auto un = dynamic_cast<UnaryExpression*>(expr)) {
            auto rt = inferType(un->right.get());
            if (un->op == "-")
                return (rt == ValueType::Float) ? ValueType::Float : ValueType::Int;
            if (un->op == "!")
                return ValueType::Bit;
            if (un->op == "~")
                return (rt == ValueType::Bit) ? ValueType::Bit : ValueType::Unknown;
            return ValueType::Unknown;
        }
        if (auto post = dynamic_cast<PostfixExpression*>(expr)) {
            if (auto v = dynamic_cast<VariableExpression*>(post->left.get()))
                return getVariableType(v->name);
            return ValueType::Unknown;
        }
        if (dynamic_cast<IndexExpression*>(expr))
            return ValueType::Unknown;  // not tracked at this level
        if (dynamic_cast<ArrayLiteralExpression*>(expr))
            return ValueType::Unknown;  // element type depends on context
        if (dynamic_cast<AssignmentExpression*>(expr))
            return ValueType::Unknown;
        if (dynamic_cast<ArrayAssignmentExpression*>(expr))
            return ValueType::Unknown;
        return ValueType::Unknown;
    }

    std::optional<int> SemanticAnalyser::evaluateConstInt(Expression* expr) const {
        if (!expr)
            return std::nullopt;

        if (auto lit = dynamic_cast<LiteralExpression*>(expr)) {
            if (lit->literalType != "int")
                return std::nullopt;
            try {
                return std::stoi(lit->value);
            } catch (...) {
                return std::nullopt;
            }
        }

        if (auto var = dynamic_cast<VariableExpression*>(expr)) {
            if (!isDeclared(var->name)) {
                throw BlochError(ErrorCategory::Semantic, var->line, var->column,
                                 "Variable '" + var->name + "' not declared");
            }
            if (!isFinal(var->name))
                return std::nullopt;
            if (getVariableType(var->name) != ValueType::Int)
                return std::nullopt;
            return m_symbols.getConstInt(var->name);
        }

        if (auto par = dynamic_cast<ParenthesizedExpression*>(expr))
            return evaluateConstInt(par->expression.get());

        if (auto unary = dynamic_cast<UnaryExpression*>(expr)) {
            if (unary->op == "-") {
                auto val = evaluateConstInt(unary->right.get());
                if (val)
                    return -*val;
            }
            return std::nullopt;
        }

        if (auto bin = dynamic_cast<BinaryExpression*>(expr)) {
            auto left = evaluateConstInt(bin->left.get());
            auto right = evaluateConstInt(bin->right.get());
            if (!left || !right)
                return std::nullopt;
            if (bin->op == "+")
                return *left + *right;
            if (bin->op == "-")
                return *left - *right;
            if (bin->op == "*")
                return *left * *right;
            if (bin->op == "/") {
                if (*right == 0) {
                    int line = bin->line > 0 ? bin->line : expr->line;
                    int col = bin->column > 0 ? bin->column : expr->column;
                    throw BlochError(ErrorCategory::Semantic, line, col,
                                     "division by zero in constant integer expression");
                }
                return *left / *right;
            }
            if (bin->op == "%") {
                if (*right == 0) {
                    int line = bin->line > 0 ? bin->line : expr->line;
                    int col = bin->column > 0 ? bin->column : expr->column;
                    throw BlochError(ErrorCategory::Semantic, line, col,
                                     "modulo by zero in constant integer expression");
                }
                return *left % *right;
            }
            return std::nullopt;
        }

        return std::nullopt;
    }

    void SemanticAnalyser::visit(VariableDeclaration& node) {
        if (isDeclared(node.name)) {
            throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                             "'" + node.name + "' is already declared in this scope");
        }
        ValueType type = ValueType::Unknown;
        if (auto prim = dynamic_cast<PrimitiveType*>(node.varType.get()))
            type = typeFromString(prim->name);
        else if (dynamic_cast<VoidType*>(node.varType.get())) {
            // Variables cannot be of type 'void'
            throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                             "variables cannot have type 'void'");
        }
        // Reject invalid annotations on variables
        for (const auto& ann : node.annotations) {
            if (ann && ann->name == "quantum") {
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "'@quantum' may annotate functions only");
            }
        }
        declare(node.name, node.isFinal, type);
        if (node.isTracked) {
            // Only 'qubit' and 'qubit[]' types can be tracked
            bool valid = false;
            if (auto prim = dynamic_cast<PrimitiveType*>(node.varType.get())) {
                valid = (prim->name == "qubit");
            } else if (auto arr = dynamic_cast<ArrayType*>(node.varType.get())) {
                if (auto elem = dynamic_cast<PrimitiveType*>(arr->elementType.get()))
                    valid = (elem->name == "qubit");
            }
            if (!valid) {
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "'@tracked' may annotate 'qubit' or 'qubit[]' only");
            }
        }
        if (auto arr = dynamic_cast<ArrayType*>(node.varType.get())) {
            bool hasExplicitSize = arr->size >= 0 || arr->sizeExpression != nullptr;
            if (arr->sizeExpression) {
                auto size = evaluateConstInt(arr->sizeExpression.get());
                if (!size) {
                    int line =
                        arr->sizeExpression->line > 0 ? arr->sizeExpression->line : node.line;
                    int col =
                        arr->sizeExpression->column > 0 ? arr->sizeExpression->column : node.column;
                    throw BlochError(ErrorCategory::Semantic, line, col,
                                     "array size must be a compile-time constant 'int' (e.g. a "
                                     "final int)");
                }
                arr->size = *size;
                hasExplicitSize = true;
            }
            if (hasExplicitSize && arr->size < 0) {
                int line = node.line;
                int col = node.column;
                if (arr->sizeExpression && arr->sizeExpression->line > 0) {
                    line = arr->sizeExpression->line;
                    col = arr->sizeExpression->column;
                }
                throw BlochError(ErrorCategory::Semantic, line, col,
                                 "array size must be non-negative");
            }
        }
        if (node.initializer) {
            // Disallow initialisation of qubit arrays
            if (auto arr = dynamic_cast<ArrayType*>(node.varType.get())) {
                if (auto elem = dynamic_cast<PrimitiveType*>(arr->elementType.get())) {
                    if (elem->name == "qubit") {
                        throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                         "qubit[] cannot be initialised");
                    }
                }
            }
            if (auto call = dynamic_cast<CallExpression*>(node.initializer.get())) {
                if (auto callee = dynamic_cast<VariableExpression*>(call->callee.get())) {
                    if (returnsVoid(callee->name)) {
                        throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                         "cannot assign result of 'void' function");
                    }
                }
            }
            // Type check: a variable initializer must match the declared variable type exactly
            /*
            TODO: explicit casting for aritmetic initializers eg
            int a = 1;
            int b = 2;
            float ratio = (float) a/b;
            */
            if (auto prim = dynamic_cast<PrimitiveType*>(node.varType.get())) {
                ValueType target = typeFromString(prim->name);
                ValueType initT = inferType(node.initializer.get());
                if (target != ValueType::Unknown && initT != ValueType::Unknown &&
                    target != initT) {
                    // Special guidance for common literal mistakes
                    if (prim->name == "bit") {
                        if (auto lit = dynamic_cast<LiteralExpression*>(node.initializer.get())) {
                            if (lit->literalType == "int") {
                                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                                 "bit literals must be 0b or 1b");
                            }
                        }
                    } else if (prim->name == "float") {
                        if (auto lit = dynamic_cast<LiteralExpression*>(node.initializer.get())) {
                            if (lit->literalType == "int") {
                                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                                 "float literals must end with 'f'");
                            }
                        }
                    }
                    throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                     "initializer for '" + node.name + "' expected '" +
                                         typeToString(target) + "' but got '" +
                                         typeToString(initT) + "'");
                }
            }
            node.initializer->accept(*this);
        }
        if (node.isFinal) {
            if (auto prim = dynamic_cast<PrimitiveType*>(node.varType.get())) {
                if (prim->name == "int" && node.initializer) {
                    auto val = evaluateConstInt(node.initializer.get());
                    if (val)
                        m_symbols.setConstInt(node.name, *val);
                }
            }
        }
    }

    void SemanticAnalyser::visit(BlockStatement& node) {
        beginScope();
        for (auto& stmt : node.statements) stmt->accept(*this);
        endScope();
    }

    void SemanticAnalyser::visit(ExpressionStatement& node) {
        if (node.expression)
            node.expression->accept(*this);
    }

    void SemanticAnalyser::visit(ReturnStatement& node) {
        m_foundReturn = true;
        bool isVoid = m_currentReturnType == ValueType::Void;
        if (node.value && isVoid) {
            throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                             "void function cannot return a value");
        }
        if (!node.value && !isVoid) {
            throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                             "Non-void function must return a value");
        }
        if (node.value) {
            // Type-check return expression when both sides are known
            ValueType actual = inferType(node.value.get());
            if (m_currentReturnType != ValueType::Unknown && actual != ValueType::Unknown &&
                actual != m_currentReturnType) {
                // Provide guidance for common literal mistakes
                if (m_currentReturnType == ValueType::Bit) {
                    if (auto lit = dynamic_cast<LiteralExpression*>(node.value.get())) {
                        if (lit->literalType == "int") {
                            throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                             "bit literals must be 0b or 1b");
                        }
                    }
                } else if (m_currentReturnType == ValueType::Float) {
                    if (auto lit = dynamic_cast<LiteralExpression*>(node.value.get())) {
                        if (lit->literalType == "int") {
                            throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                             "float literals must end with 'f'");
                        }
                    }
                }
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "return type mismatch: expected '" +
                                     typeToString(m_currentReturnType) + "' but got '" +
                                     typeToString(actual) + "'");
            }
            node.value->accept(*this);
        }
    }

    void SemanticAnalyser::visit(IfStatement& node) {
        if (node.condition)
            node.condition->accept(*this);
        if (node.thenBranch)
            node.thenBranch->accept(*this);
        if (node.elseBranch)
            node.elseBranch->accept(*this);
    }

    void SemanticAnalyser::visit(TernaryStatement& node) {
        if (node.condition)
            node.condition->accept(*this);
        if (node.thenBranch)
            node.thenBranch->accept(*this);
        if (node.elseBranch)
            node.elseBranch->accept(*this);
    }

    void SemanticAnalyser::visit(ForStatement& node) {
        beginScope();
        if (node.initializer)
            node.initializer->accept(*this);
        if (node.condition)
            node.condition->accept(*this);
        if (node.increment)
            node.increment->accept(*this);
        if (node.body)
            node.body->accept(*this);
        endScope();
    }

    void SemanticAnalyser::visit(WhileStatement& node) {
        if (node.condition)
            node.condition->accept(*this);
        if (node.body)
            node.body->accept(*this);
    }

    void SemanticAnalyser::visit(EchoStatement& node) {
        if (node.value)
            node.value->accept(*this);
    }

    void SemanticAnalyser::visit(ResetStatement& node) {
        if (node.target) {
            // Best-effort type validation for simple variable targets
            if (auto var = dynamic_cast<VariableExpression*>(node.target.get())) {
                ValueType t = getVariableType(var->name);
                if (t != ValueType::Unknown && t != ValueType::Qubit) {
                    throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                     "reset target must be a 'qubit'");
                }
            }
            node.target->accept(*this);
        }
    }

    void SemanticAnalyser::visit(MeasureStatement& node) {
        if (node.qubit) {
            // Best-effort type validation for simple variable targets
            if (auto var = dynamic_cast<VariableExpression*>(node.qubit.get())) {
                ValueType t = getVariableType(var->name);
                if (t != ValueType::Unknown && t != ValueType::Qubit) {
                    throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                     "measure target must be a 'qubit'");
                }
            }
            node.qubit->accept(*this);
        }
    }

    void SemanticAnalyser::visit(AssignmentStatement& node) {
        if (!isDeclared(node.name)) {
            throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                             "Variable '" + node.name + "' not declared");
        }
        if (isFinal(node.name)) {
            throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                             "Cannot assign to final variable '" + node.name + "'");
        }
        if (node.value) {
            if (auto call = dynamic_cast<CallExpression*>(node.value.get())) {
                if (auto callee = dynamic_cast<VariableExpression*>(call->callee.get())) {
                    if (returnsVoid(callee->name)) {
                        throw BlochError(node.line, node.column,
                                         "Cannot assign result of void function");
                    }
                }
            }
            node.value->accept(*this);
        }
    }

    void SemanticAnalyser::visit(BinaryExpression& node) {
        if (node.left)
            node.left->accept(*this);
        if (node.right)
            node.right->accept(*this);
    }

    void SemanticAnalyser::visit(UnaryExpression& node) {
        if (node.right)
            node.right->accept(*this);
    }

    void SemanticAnalyser::visit(PostfixExpression& node) {
        if (auto var = dynamic_cast<VariableExpression*>(node.left.get())) {
            if (!isDeclared(var->name)) {
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "Variable '" + var->name + "' not declared");
            }
            if (isFinal(var->name)) {
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "Cannot modify final variable '" + var->name + "'");
            }
            // Postfix operators are only valid for integer variables
            ValueType t = getVariableType(var->name);
            if (t != ValueType::Int) {
                throw BlochError(
                    ErrorCategory::Semantic, node.line, node.column,
                    "Postfix operator '" + node.op + "' requires variable of type 'int'");
            }
        } else if (node.left) {
            // Only variables are valid lvalues for postfix operators
            node.left->accept(*this);
            throw BlochError(
                ErrorCategory::Semantic, node.line, node.column,
                "Postfix operator '" + node.op + "' can only be applied to a variable");
        }
    }

    void SemanticAnalyser::visit(LiteralExpression&) {}

    void SemanticAnalyser::visit(VariableExpression& node) {
        if (!isDeclared(node.name)) {
            throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                             "Variable '" + node.name + "' not declared");
        }
    }

    void SemanticAnalyser::visit(CallExpression& node) {
        if (auto var = dynamic_cast<VariableExpression*>(node.callee.get())) {
            if (!isDeclared(var->name) && !isFunctionDeclared(var->name)) {
                throw BlochError(ErrorCategory::Semantic, var->line, var->column,
                                 "Variable '" + var->name + "' not declared");
            }
            size_t expected = getFunctionParamCount(var->name);
            if (expected != node.arguments.size()) {
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "Function '" + var->name + "' expects " +
                                     std::to_string(expected) + " argument(s)");
            }
            auto types = getFunctionParamTypes(var->name);
            for (size_t i = 0; i < node.arguments.size() && i < types.size(); ++i) {
                auto& arg = node.arguments[i];
                ValueType expectedType = types[i];
                if (expectedType == ValueType::Unknown)
                    continue;
                if (auto argVar = dynamic_cast<VariableExpression*>(arg.get())) {
                    ValueType actual = getVariableType(argVar->name);
                    if (actual != ValueType::Unknown && actual != expectedType) {
                        std::string actualStr = typeToString(actual);
                        throw BlochError(ErrorCategory::Semantic, argVar->line, argVar->column,
                                         "argument #" + std::to_string(i + 1) + " to " + var->name +
                                             " expected '" + typeToString(expectedType) +
                                             "' but got '" + actualStr + "'");
                    }
                } else if (auto argLit = dynamic_cast<LiteralExpression*>(arg.get())) {
                    ValueType actual = typeFromString(argLit->literalType);
                    if (actual != ValueType::Unknown && actual != expectedType) {
                        std::string actualStr = typeToString(actual);
                        throw BlochError(ErrorCategory::Semantic, argLit->line, argLit->column,
                                         "argument #" + std::to_string(i + 1) + " to " + var->name +
                                             " expected '" + typeToString(expectedType) +
                                             "' but got '" + actualStr + "'");
                    }
                } else {
                    // Fallback: check any other expression if we can infer its type
                    ValueType actual = inferType(arg.get());
                    if (actual != ValueType::Unknown && actual != expectedType) {
                        throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                         "argument #" + std::to_string(i + 1) + " to " + var->name +
                                             " expected '" + typeToString(expectedType) +
                                             "' but got '" + typeToString(actual) + "'");
                    }
                }
            }
        } else if (node.callee) {
            node.callee->accept(*this);
        }
        for (auto& arg : node.arguments) arg->accept(*this);
    }

    void SemanticAnalyser::visit(ArrayLiteralExpression& node) {
        for (auto& elem : node.elements) {
            if (elem)
                elem->accept(*this);
        }
    }

    void SemanticAnalyser::visit(IndexExpression& node) {
        if (node.collection)
            node.collection->accept(*this);
        if (node.index)
            node.index->accept(*this);
    }

    void SemanticAnalyser::visit(ParenthesizedExpression& node) {
        if (node.expression)
            node.expression->accept(*this);
    }

    void SemanticAnalyser::visit(MeasureExpression& node) {
        if (node.qubit) {
            // Best-effort type validation for simple variable targets
            if (auto var = dynamic_cast<VariableExpression*>(node.qubit.get())) {
                ValueType t = getVariableType(var->name);
                if (t != ValueType::Unknown && t != ValueType::Qubit) {
                    throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                     "measure target must be a 'qubit'");
                }
            }
            node.qubit->accept(*this);
        }
    }

    void SemanticAnalyser::visit(AssignmentExpression& node) {
        if (!isDeclared(node.name)) {
            throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                             "Variable '" + node.name + "' not declared");
        }
        if (isFinal(node.name)) {
            throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                             "Cannot assign to final variable '" + node.name + "'");
        }
        if (node.value) {
            if (auto call = dynamic_cast<CallExpression*>(node.value.get())) {
                if (auto callee = dynamic_cast<VariableExpression*>(call->callee.get())) {
                    if (returnsVoid(callee->name)) {
                        throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                         "cannot assign result of 'void' function");
                    }
                }
            }
            // Type check assignment expression
            ValueType target = getVariableType(node.name);
            ValueType source = inferType(node.value.get());
            if (target != ValueType::Unknown && source != ValueType::Unknown && target != source) {
                // Special guidance
                if (target == ValueType::Bit) {
                    if (auto lit = dynamic_cast<LiteralExpression*>(node.value.get())) {
                        if (lit->literalType == "int") {
                            throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                             "bit literals must be 0b or 1b");
                        }
                    }
                } else if (target == ValueType::Float) {
                    if (auto lit = dynamic_cast<LiteralExpression*>(node.value.get())) {
                        if (lit->literalType == "int") {
                            throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                             "float literals must end with 'f'");
                        }
                    }
                }
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "assignment to '" + node.name + "' expected '" +
                                     typeToString(target) + "' but got '" + typeToString(source) +
                                     "'");
            }
            node.value->accept(*this);
        }
    }

    void SemanticAnalyser::visit(ArrayAssignmentExpression& node) {
        // Validate collection is a declared, non-final variable
        if (auto var = dynamic_cast<VariableExpression*>(node.collection.get())) {
            if (!isDeclared(var->name)) {
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "Variable '" + var->name + "' not declared");
            }
            if (isFinal(var->name)) {
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "Cannot assign to final variable '" + var->name + "'");
            }
        } else {
            // Only simple variable arrays are assignable targets for now
            node.collection->accept(*this);
            throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                             "Assignment target must be a variable array");
        }
        if (node.index)
            node.index->accept(*this);
        if (node.value)
            node.value->accept(*this);
    }

    void SemanticAnalyser::visit(PrimitiveType&) {}
    void SemanticAnalyser::visit(ArrayType&) {}
    void SemanticAnalyser::visit(VoidType&) {}

    void SemanticAnalyser::visit(Parameter& node) {
        if (node.type)
            node.type->accept(*this);
    }

    void SemanticAnalyser::visit(AnnotationNode&) {}

    void SemanticAnalyser::visit(FunctionDeclaration& node) {
        if (node.hasQuantumAnnotation) {
            bool valid = false;
            if (dynamic_cast<VoidType*>(node.returnType.get())) {
                valid = true;
            } else if (auto prim = dynamic_cast<PrimitiveType*>(node.returnType.get())) {
                if (prim->name == "bit")
                    valid = true;
            }
            if (!valid) {
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "'@quantum' functions must return 'bit' or 'void'");
            }
        }
        ValueType prevReturn = m_currentReturnType;
        if (auto prim = dynamic_cast<PrimitiveType*>(node.returnType.get()))
            m_currentReturnType = typeFromString(prim->name);
        else if (dynamic_cast<VoidType*>(node.returnType.get()))
            m_currentReturnType = ValueType::Void;
        else
            m_currentReturnType = ValueType::Unknown;

        FunctionInfo info;
        info.returnType = m_currentReturnType;
        for (auto& param : node.params) {
            if (auto prim = dynamic_cast<PrimitiveType*>(param->type.get()))
                info.paramTypes.push_back(typeFromString(prim->name));
            else if (dynamic_cast<VoidType*>(param->type.get()))
                info.paramTypes.push_back(ValueType::Void);
            else
                info.paramTypes.push_back(ValueType::Unknown);
        }
        m_functionInfo[node.name] = info;

        bool prevFoundReturn = m_foundReturn;
        m_foundReturn = false;
        beginScope();
        for (auto& param : node.params) {
            if (isDeclared(param->name)) {
                throw BlochError(ErrorCategory::Semantic, param->line, param->column,
                                 "'" + param->name + "' is already declared in this scope");
            }
            ValueType type = ValueType::Unknown;
            if (dynamic_cast<VoidType*>(param->type.get())) {
                throw BlochError(ErrorCategory::Semantic, param->line, param->column,
                                 "parameters cannot have type 'void'");
            } else if (auto prim = dynamic_cast<PrimitiveType*>(param->type.get()))
                type = typeFromString(prim->name);
            declare(param->name, false, type);
            param->accept(*this);
        }
        if (node.body)
            node.body->accept(*this);
        if (m_currentReturnType != ValueType::Void && !m_foundReturn) {
            throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                             "Non-void function must have a 'return' statement.");
        }
        endScope();

        m_foundReturn = prevFoundReturn;
        m_currentReturnType = prevReturn;
    }

    void SemanticAnalyser::visit(Program& node) {
        for (auto& fn : node.functions) {
            if (isFunctionDeclared(fn->name)) {
                throw BlochError(ErrorCategory::Semantic, fn->line, fn->column,
                                 "'" + fn->name + "' is already declared in this scope");
            }
            declareFunction(fn->name);
        }
        for (auto& fn : node.functions) fn->accept(*this);
        for (auto& stmt : node.statements) stmt->accept(*this);
    }

    void SemanticAnalyser::beginScope() { m_symbols.beginScope(); }

    void SemanticAnalyser::endScope() { m_symbols.endScope(); }

    void SemanticAnalyser::declare(const std::string& name, bool isFinalVar, ValueType type) {
        m_symbols.declare(name, isFinalVar, type);
    }

    bool SemanticAnalyser::isDeclared(const std::string& name) const {
        return m_symbols.isDeclared(name);
    }

    void SemanticAnalyser::declareFunction(const std::string& name) { m_functions.insert(name); }

    bool SemanticAnalyser::isFunctionDeclared(const std::string& name) const {
        return m_functions.count(name) > 0 || builtInGates.count(name) > 0;
    }

    bool SemanticAnalyser::isFinal(const std::string& name) const {
        return m_symbols.isFinal(name);
    }

    size_t SemanticAnalyser::getFunctionParamCount(const std::string& name) const {
        auto it = m_functionInfo.find(name);
        if (it != m_functionInfo.end())
            return it->second.paramTypes.size();
        auto builtin = builtInGates.find(name);
        if (builtin != builtInGates.end())
            return builtin->second.paramTypes.size();
        return 0;
    }

    std::vector<ValueType> SemanticAnalyser::getFunctionParamTypes(const std::string& name) const {
        auto it = m_functionInfo.find(name);
        if (it != m_functionInfo.end())
            return it->second.paramTypes;
        auto builtin = builtInGates.find(name);
        if (builtin != builtInGates.end())
            return builtin->second.paramTypes;
        return {};
    }

    ValueType SemanticAnalyser::getVariableType(const std::string& name) const {
        return m_symbols.getType(name);
    }

    bool SemanticAnalyser::returnsVoid(const std::string& name) const {
        auto it = m_functionInfo.find(name);
        if (it != m_functionInfo.end())
            return it->second.returnType == ValueType::Void;
        auto builtin = builtInGates.find(name);
        if (builtin != builtInGates.end())
            return builtin->second.returnType == ValueType::Void;
        return false;
    }

}  // namespace bloch::core
