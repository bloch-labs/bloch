#include "semantic_analyser.hpp"

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
        declare(node.name, node.isFinal);
        if (node.initializer)
            node.initializer->accept(*this);
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
        bool isVoid = dynamic_cast<VoidType*>(m_currentReturnType) != nullptr;
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
        if (node.value)
            node.value->accept(*this);
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

    void SemanticAnalyser::visit(LiteralExpression&) {}

    void SemanticAnalyser::visit(VariableExpression& node) {
        if (!isDeclared(node.name)) {
            throw BlochRuntimeError("Bloch Semantic Error", node.line, node.column,
                                    "Variable '" + node.name + "' not declared");
        }
    }

    void SemanticAnalyser::visit(CallExpression& node) {
        if (node.callee)
            node.callee->accept(*this);
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
        if (node.value)
            node.value->accept(*this);
    }

    void SemanticAnalyser::visit(ConstructorCallExpression& node) {
        for (auto& arg : node.arguments) arg->accept(*this);
    }

    void SemanticAnalyser::visit(MemberAccessExpression& node) {
        if (node.object)
            node.object->accept(*this);
    }

    void SemanticAnalyser::visit(PrimitiveType&) {}
    void SemanticAnalyser::visit(LogicalType&) {}
    void SemanticAnalyser::visit(ArrayType&) {}
    void SemanticAnalyser::visit(VoidType&) {}
    void SemanticAnalyser::visit(ObjectType&) {}

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
        Type* prevReturn = m_currentReturnType;
        m_currentReturnType = node.returnType.get();

        beginScope();
        for (auto& param : node.params) {
            if (isDeclared(param->name)) {
                throw BlochRuntimeError("Bloch Semantic Error", param->line, param->column,
                                        "Parameter '" + param->name + "' redeclared");
            }
            declare(param->name, false);
            param->accept(*this);
        }
        if (node.body)
            node.body->accept(*this);
        endScope();

        m_currentReturnType = prevReturn;
    }

    void SemanticAnalyser::visit(ClassDeclaration& node) {
        for (auto& member : node.members) member->accept(*this);
        for (auto& method : node.methods) method->accept(*this);
    }

    void SemanticAnalyser::visit(Program& node) {
        for (auto& imp : node.imports) imp->accept(*this);
        for (auto& fn : node.functions) fn->accept(*this);
        for (auto& cls : node.classes) cls->accept(*this);
        for (auto& stmt : node.statements) stmt->accept(*this);
    }

    void SemanticAnalyser::beginScope() { m_scopes.emplace_back(); }

    void SemanticAnalyser::endScope() { m_scopes.pop_back(); }

    void SemanticAnalyser::declare(const std::string& name, bool isFinalVar) {
        if (m_scopes.empty())
            return;
        m_scopes.back()[name] = VariableInfo{isFinalVar};
    }

    bool SemanticAnalyser::isDeclared(const std::string& name) const {
        for (auto it = m_scopes.rbegin(); it != m_scopes.rend(); ++it) {
            if (it->count(name))
                return true;
        }
        return false;
    }

    bool SemanticAnalyser::isFinal(const std::string& name) const {
        for (auto it = m_scopes.rbegin(); it != m_scopes.rend(); ++it) {
            auto found = it->find(name);
            if (found != it->end())
                return found->second.isFinal;
        }
        return false;
    }

}