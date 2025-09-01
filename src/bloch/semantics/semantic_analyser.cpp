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

#include "semantic_analyser.hpp"
#include <iostream>
#include "built_ins.hpp"

namespace bloch {

    void SemanticAnalyser::analyse(Program& program) {
        // Global scope for top-level declarations and functions.
        beginScope();
        program.accept(*this);
        endScope();
    }

    void SemanticAnalyser::visit(VariableDeclaration& node) {
        if (isDeclared(node.name)) {
            throw BlochError(node.line, node.column, "Variable '" + node.name + "' redeclared");
        }
        ValueType type = ValueType::Unknown;
        if (auto prim = dynamic_cast<PrimitiveType*>(node.varType.get()))
            type = typeFromString(prim->name);
        else if (dynamic_cast<VoidType*>(node.varType.get()))
            type = ValueType::Void;
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
                throw BlochError(node.line, node.column,
                                 "@tracked variables must be qubit or qubit[]");
            }
        }
        if (node.initializer) {
            // Disallow initialisation of qubit arrays
            if (auto arr = dynamic_cast<ArrayType*>(node.varType.get())) {
                if (auto elem = dynamic_cast<PrimitiveType*>(arr->elementType.get())) {
                    if (elem->name == "qubit") {
                        throw BlochError(node.line, node.column, "qubit[] cannot be initialised");
                    }
                }
            }
            if (auto call = dynamic_cast<CallExpression*>(node.initializer.get())) {
                if (auto callee = dynamic_cast<VariableExpression*>(call->callee.get())) {
                    if (returnsVoid(callee->name)) {
                        throw BlochError(node.line, node.column,
                                         "Cannot assign result of void function");
                    }
                }
            }
            node.initializer->accept(*this);
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
            throw BlochError(node.line, node.column, "Void function cannot return a value");
        }
        if (!node.value && !isVoid) {
            throw BlochError(node.line, node.column, "Non-void function must return a value");
        }
        if (node.value)
            node.value->accept(*this);
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
        if (node.target)
            node.target->accept(*this);
    }

    void SemanticAnalyser::visit(MeasureStatement& node) {
        if (node.qubit)
            node.qubit->accept(*this);
    }

    void SemanticAnalyser::visit(AssignmentStatement& node) {
        if (!isDeclared(node.name)) {
            throw BlochError(node.line, node.column, "Variable '" + node.name + "' not declared");
        }
        if (isFinal(node.name)) {
            throw BlochError(node.line, node.column,
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
                throw BlochError(node.line, node.column,
                                 "Variable '" + var->name + "' not declared");
            }
            if (isFinal(var->name)) {
                throw BlochError(node.line, node.column,
                                 "Cannot modify final variable '" + var->name + "'");
            }
            // Postfix operators are only valid for integer variables
            ValueType t = getVariableType(var->name);
            if (t != ValueType::Int) {
                throw BlochError(
                    node.line, node.column,
                    "Postfix operator '" + node.op + "' requires variable of type 'int'");
            }
        } else if (node.left) {
            // Only variables are valid lvalues for postfix operators
            node.left->accept(*this);
            throw BlochError(
                node.line, node.column,
                "Postfix operator '" + node.op + "' can only be applied to a variable");
        }
    }

    void SemanticAnalyser::visit(LiteralExpression&) {}

    void SemanticAnalyser::visit(VariableExpression& node) {
        if (!isDeclared(node.name)) {
            throw BlochError(node.line, node.column, "Variable '" + node.name + "' not declared");
        }
    }

    void SemanticAnalyser::visit(CallExpression& node) {
        if (auto var = dynamic_cast<VariableExpression*>(node.callee.get())) {
            if (!isDeclared(var->name) && !isFunctionDeclared(var->name)) {
                throw BlochError(var->line, var->column,
                                 "Variable '" + var->name + "' not declared");
            }
            size_t expected = getFunctionParamCount(var->name);
            if (expected != node.arguments.size()) {
                throw BlochError(node.line, node.column,
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
                        throw BlochError(argVar->line, argVar->column,
                                         "Argument " + std::to_string(i + 1) + " of '" + var->name +
                                             "' expects type '" + typeToString(expectedType) + "'");
                    }
                } else if (auto argLit = dynamic_cast<LiteralExpression*>(arg.get())) {
                    ValueType actual = typeFromString(argLit->literalType);
                    if (actual != ValueType::Unknown && actual != expectedType) {
                        throw BlochError(argLit->line, argLit->column,
                                         "Argument " + std::to_string(i + 1) + " of '" + var->name +
                                             "' expects type '" + typeToString(expectedType) + "'");
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
        if (node.qubit)
            node.qubit->accept(*this);
    }

    void SemanticAnalyser::visit(AssignmentExpression& node) {
        if (!isDeclared(node.name)) {
            throw BlochError(node.line, node.column, "Variable '" + node.name + "' not declared");
        }
        if (isFinal(node.name)) {
            throw BlochError(node.line, node.column,
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

    void SemanticAnalyser::visit(ArrayAssignmentExpression& node) {
        // Validate collection is a declared, non-final variable
        if (auto var = dynamic_cast<VariableExpression*>(node.collection.get())) {
            if (!isDeclared(var->name)) {
                throw BlochError(node.line, node.column,
                                 "Variable '" + var->name + "' not declared");
            }
            if (isFinal(var->name)) {
                throw BlochError(node.line, node.column,
                                 "Cannot assign to final variable '" + var->name + "'");
            }
        } else {
            // Only simple variable arrays are assignable targets for now
            node.collection->accept(*this);
            throw BlochError(node.line, node.column, "Assignment target must be a variable array");
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
                throw BlochError(node.line, node.column,
                                 "@quantum functions must return 'bit' or 'void'");
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
                throw BlochError(param->line, param->column,
                                 "Parameter '" + param->name + "' redeclared");
            }
            ValueType type = ValueType::Unknown;
            if (auto prim = dynamic_cast<PrimitiveType*>(param->type.get()))
                type = typeFromString(prim->name);
            else if (dynamic_cast<VoidType*>(param->type.get()))
                type = ValueType::Void;
            declare(param->name, false, type);
            param->accept(*this);
        }
        if (node.body)
            node.body->accept(*this);
        if (m_currentReturnType != ValueType::Void && !m_foundReturn) {
            throw BlochError(node.line, node.column,
                             "Non-void function must have a 'return' statement.");
        }
        endScope();

        m_foundReturn = prevFoundReturn;
        m_currentReturnType = prevReturn;
    }

    void SemanticAnalyser::visit(Program& node) {
        for (auto& fn : node.functions) {
            if (isFunctionDeclared(fn->name)) {
                throw BlochError(fn->line, fn->column, "Function '" + fn->name + "' redeclared");
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

}
