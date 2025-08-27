#include "semantic_analyser.hpp"
#include <iostream>
#include "built_ins.hpp"

namespace bloch {

    void SemanticAnalyser::analyse(Program& program) {
        beginScope();
        program.accept(*this);
        endScope();
    }

    void SemanticAnalyser::visit(ImportStatement&) {}

    void SemanticAnalyser::visit(VariableDeclaration& node) {
        if (isDeclared(node.name)) {
            throw BlochRuntimeError("Bloch Semantic Error", node.line, node.column,
                                    "Variable '" + node.name + "' redeclared");
        }
        ValueType type = ValueType::Unknown;
        if (auto prim = dynamic_cast<PrimitiveType*>(node.varType.get()))
            type = typeFromString(prim->name);
        else if (dynamic_cast<VoidType*>(node.varType.get()))
            type = ValueType::Void;
        declare(node.name, node.isFinal, type);
        if (node.isTracked) {
            if (type != ValueType::Bit && type != ValueType::Int) {
                throw BlochRuntimeError("Bloch Semantic Error", node.line, node.column,
                                        "@tracked variables must be bit or int");
            }
            if (!node.initializer) {
                std::cerr << "[WARN] tracked variable '" << node.name << "' may be uninitialized\n";
            }
        }
        if (node.initializer) {
            if (auto call = dynamic_cast<CallExpression*>(node.initializer.get())) {
                if (auto callee = dynamic_cast<VariableExpression*>(call->callee.get())) {
                    if (returnsVoid(callee->name)) {
                        throw BlochRuntimeError("Bloch Semantic Error", node.line, node.column,
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
            throw BlochRuntimeError("Bloch Semantic Error", node.line, node.column,
                                    "Void function cannot return a value");
        }
        if (!node.value && !isVoid) {
            throw BlochRuntimeError("Bloch Semantic Error", node.line, node.column,
                                    "Non-void function must return a value");
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
            throw BlochRuntimeError("Bloch Semantic Error", node.line, node.column,
                                    "Variable '" + node.name + "' not declared");
        }
        if (isFinal(node.name)) {
            throw BlochRuntimeError("Bloch Semantic Error", node.line, node.column,
                                    "Cannot assign to final variable '" + node.name + "'");
        }
        if (node.value) {
            if (auto call = dynamic_cast<CallExpression*>(node.value.get())) {
                if (auto callee = dynamic_cast<VariableExpression*>(call->callee.get())) {
                    if (returnsVoid(callee->name)) {
                        throw BlochRuntimeError("Bloch Semantic Error", node.line, node.column,
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
                throw BlochRuntimeError("Bloch Semantic Error", node.line, node.column,
                                        "Variable '" + var->name + "' not declared");
            }
            if (isFinal(var->name)) {
                throw BlochRuntimeError("Bloch Semantic Error", node.line, node.column,
                                        "Cannot modify final variable '" + var->name + "'");
            }
        } else if (node.left) {
            node.left->accept(*this);
        }
    }

    void SemanticAnalyser::visit(LiteralExpression&) {}

    void SemanticAnalyser::visit(VariableExpression& node) {
        if (!isDeclared(node.name)) {
            throw BlochRuntimeError("Bloch Semantic Error", node.line, node.column,
                                    "Variable '" + node.name + "' not declared");
        }
    }

    void SemanticAnalyser::visit(CallExpression& node) {
        if (auto var = dynamic_cast<VariableExpression*>(node.callee.get())) {
            if (!isDeclared(var->name) && !isFunctionDeclared(var->name)) {
                throw BlochRuntimeError("Bloch Semantic Error", var->line, var->column,
                                        "Variable '" + var->name + "' not declared");
            }
            size_t expected = getFunctionParamCount(var->name);
            if (expected != node.arguments.size()) {
                throw BlochRuntimeError("Bloch Semantic Error", node.line, node.column,
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
                        throw BlochRuntimeError(
                            "Bloch Semantic Error", argVar->line, argVar->column,
                            "Argument " + std::to_string(i + 1) + " of '" + var->name +
                                "' expects type '" + typeToString(expectedType) + "'");
                    }
                } else if (auto argLit = dynamic_cast<LiteralExpression*>(arg.get())) {
                    ValueType actual = typeFromString(argLit->literalType);
                    if (actual != ValueType::Unknown && actual != expectedType) {
                        throw BlochRuntimeError(
                            "Bloch Semantic Error", argLit->line, argLit->column,
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
            throw BlochRuntimeError("Bloch Semantic Error", node.line, node.column,
                                    "Variable '" + node.name + "' not declared");
        }
        if (isFinal(node.name)) {
            throw BlochRuntimeError("Bloch Semantic Error", node.line, node.column,
                                    "Cannot assign to final variable '" + node.name + "'");
        }
        if (node.value) {
            if (auto call = dynamic_cast<CallExpression*>(node.value.get())) {
                if (auto callee = dynamic_cast<VariableExpression*>(call->callee.get())) {
                    if (returnsVoid(callee->name)) {
                        throw BlochRuntimeError("Bloch Semantic Error", node.line, node.column,
                                                "Cannot assign result of void function");
                    }
                }
            }
            node.value->accept(*this);
        }
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
                throw BlochRuntimeError("Bloch Semantic Error", node.line, node.column,
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
                throw BlochRuntimeError("Bloch Semantic Error", param->line, param->column,
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
            throw BlochRuntimeError("Bloch Semantic Error", node.line, node.column,
                                    "Non-void function must have a 'return' statement.");
        }
        endScope();

        m_foundReturn = prevFoundReturn;
        m_currentReturnType = prevReturn;
    }

    void SemanticAnalyser::visit(Program& node) {
        for (auto& fn : node.functions) {
            if (isFunctionDeclared(fn->name)) {
                throw BlochRuntimeError("Bloch Semantic Error", fn->line, fn->column,
                                        "Function '" + fn->name + "' redeclared");
            }
            declareFunction(fn->name);
        }
        for (auto& imp : node.imports) imp->accept(*this);
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