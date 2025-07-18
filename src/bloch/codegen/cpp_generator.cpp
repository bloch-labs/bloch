#include "cpp_generator.hpp"
#include <sstream>
#include <stdexcept>

namespace bloch {

    void CppGenerator::indent() { m_code.append(m_indent * 4, ' '); }

    std::string CppGenerator::cppType(Type* t) {
        if (!t)
            return "void";
        if (auto prim = dynamic_cast<PrimitiveType*>(t)) {
            if (prim->name == "bit")
                return "bool";
            if (prim->name == "qubit")
                return "/*qubit*/";  // should not appear in generated code
            return prim->name;
        }
        if (dynamic_cast<VoidType*>(t))
            return "void";
        return "int";
    }

    void CppGenerator::genFunction(FunctionDeclaration* fn) {
        indent();
        m_code += cppType(fn->returnType.get()) + " " + fn->name + "(";
        for (size_t i = 0; i < fn->params.size(); ++i) {
            if (i)
                m_code += ", ";
            m_code += cppType(fn->params[i]->type.get()) + " " + fn->params[i]->name;
        }
        m_code += ")\n";
        genStmt(fn->body.get());
        m_code += "\n";
    }

    std::string CppGenerator::genExpr(Expression* e) {
        if (!e)
            return "";
        if (auto lit = dynamic_cast<LiteralExpression*>(e)) {
            return lit->value;
        } else if (auto var = dynamic_cast<VariableExpression*>(e)) {
            return var->name;
        } else if (auto bin = dynamic_cast<BinaryExpression*>(e)) {
            return genExpr(bin->left.get()) + " " + bin->op + " " + genExpr(bin->right.get());
        } else if (auto unary = dynamic_cast<UnaryExpression*>(e)) {
            return unary->op + genExpr(unary->right.get());
        } else if (auto call = dynamic_cast<CallExpression*>(e)) {
            if (auto var = dynamic_cast<VariableExpression*>(call->callee.get())) {
                const std::string& name = var->name;
                if (builtInGates.count(name))
                    return "";  // omit quantum builtins
            }
            std::ostringstream oss;
            oss << genExpr(call->callee.get()) << "(";
            for (size_t i = 0; i < call->arguments.size(); ++i) {
                if (i)
                    oss << ", ";
                oss << genExpr(call->arguments[i].get());
            }
            oss << ")";
            auto it = m_measure.find(e);
            if (it != m_measure.end()) {
                return it->second ? "true" : "false";
            }
            return oss.str();
        } else if (auto meas = dynamic_cast<MeasureExpression*>(e)) {
            auto it = m_measure.find(e);
            if (it != m_measure.end())
                return it->second ? "true" : "false";
            return "false";
        } else if (auto assign = dynamic_cast<AssignmentExpression*>(e)) {
            return assign->name + " = " + genExpr(assign->value.get());
        }
        return "";
    }

    void CppGenerator::genStmt(Statement* s) {
        if (!s)
            return;
        if (auto var = dynamic_cast<VariableDeclaration*>(s)) {
            if (auto prim = dynamic_cast<PrimitiveType*>(var->varType.get())) {
                if (prim->name == "qubit")
                    return;  // omit quantum
                indent();
                m_code += cppType(prim) + std::string(" ") + var->name;
                if (var->initializer)
                    m_code += " = " + genExpr(var->initializer.get());
                m_code += ";\n";
            }
        } else if (auto exprs = dynamic_cast<ExpressionStatement*>(s)) {
            std::string e = genExpr(exprs->expression.get());
            if (!e.empty()) {
                indent();
                m_code += e + ";\n";
            }
        } else if (auto ret = dynamic_cast<ReturnStatement*>(s)) {
            indent();
            m_code += "return";
            if (ret->value)
                m_code += " " + genExpr(ret->value.get());
            m_code += ";\n";
        } else if (auto block = dynamic_cast<BlockStatement*>(s)) {
            indent();
            m_code += "{\n";
            ++m_indent;
            for (auto& st : block->statements) genStmt(st.get());
            --m_indent;
            indent();
            m_code += "}\n";
        } else if (auto ifs = dynamic_cast<IfStatement*>(s)) {
            indent();
            m_code += "if (" + genExpr(ifs->condition.get()) + ")\n";
            genStmt(ifs->thenBranch.get());
            if (ifs->elseBranch) {
                indent();
                m_code += "else\n";
                genStmt(ifs->elseBranch.get());
            }
        } else if (auto fors = dynamic_cast<ForStatement*>(s)) {
            indent();
            m_code += "for (";
            if (auto vd = dynamic_cast<VariableDeclaration*>(fors->initializer.get())) {
                if (auto prim = dynamic_cast<PrimitiveType*>(vd->varType.get())) {
                    m_code += cppType(prim) + " " + vd->name;
                    if (vd->initializer)
                        m_code += " = " + genExpr(vd->initializer.get());
                }
            } else if (auto es = dynamic_cast<ExpressionStatement*>(fors->initializer.get())) {
                m_code += genExpr(es->expression.get());
            }
            m_code += "; ";
            m_code += genExpr(fors->condition.get());
            m_code += "; ";
            m_code += genExpr(fors->increment.get());
            m_code += ")\n";
            genStmt(fors->body.get());
        } else if (auto echo = dynamic_cast<EchoStatement*>(s)) {
            indent();
            m_code += "std::cout << " + genExpr(echo->value.get()) + " << std::endl;\n";
        } else if (auto assign = dynamic_cast<AssignmentStatement*>(s)) {
            indent();
            m_code += assign->name + " = " + genExpr(assign->value.get()) + ";\n";
        }
    }

    bool CppGenerator::containsQubit(Type* t) {
        if (!t)
            return false;
        if (auto prim = dynamic_cast<PrimitiveType*>(t)) {
            return prim->name == "qubit";
        } else if (auto arr = dynamic_cast<ArrayType*>(t)) {
            return containsQubit(arr->elementType.get());
        }
        return false;
    }

    bool CppGenerator::shouldSkipFunction(FunctionDeclaration* fn) {
        if (!fn)
            return true;
        if (fn->hasQuantumAnnotation)
            return true;
        if (containsQubit(fn->returnType.get()))
            return true;
        for (auto& p : fn->params) {
            if (containsQubit(p->type.get()))
                return true;
        }
        return false;
    }

    std::string CppGenerator::generate(Program& program) {
        m_code += "#include <iostream>\n";
        for (auto& fn : program.functions) {
            if (fn->name != "main" && !shouldSkipFunction(fn.get())) {
                genFunction(fn.get());
            }
        }
        for (auto& fn : program.functions) {
            if (fn->name == "main") {
                m_code += "int main()\n";
                genStmt(fn->body.get());
            }
        }
        return m_code;
    }

}  // namespace bloch