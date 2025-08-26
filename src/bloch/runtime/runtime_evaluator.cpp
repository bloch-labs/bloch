#include "runtime_evaluator.hpp"
#include <sstream>
#include "../error/bloch_runtime_error.hpp"
#include "../semantics/built_ins.hpp"

namespace bloch {

    static std::string valueToString(const Value& v) {
        std::ostringstream oss;
        switch (v.type) {
            case Value::Type::String:
                return v.stringValue;
            case Value::Type::Float:
                oss << v.floatValue;
                return oss.str();
            case Value::Type::Bit:
                return std::to_string(v.bitValue);
            case Value::Type::Int:
                return std::to_string(v.intValue);
            default:
                return "";
        }
    }

    void RuntimeEvaluator::execute(Program& program) {
        for (auto& fn : program.functions) {
            m_functions[fn->name] = fn.get();
        }
        auto it = m_functions.find("main");
        if (it != m_functions.end()) {
            call(it->second, {});
        }
        warnUnmeasured();
    }

    Value RuntimeEvaluator::lookup(const std::string& name) {
        for (auto it = m_env.rbegin(); it != m_env.rend(); ++it) {
            auto fit = it->find(name);
            if (fit != it->end())
                return fit->second.value;
        }
        return {};
    }

    void RuntimeEvaluator::assign(const std::string& name, const Value& v) {
        for (auto it = m_env.rbegin(); it != m_env.rend(); ++it) {
            auto fit = it->find(name);
            if (fit != it->end()) {
                fit->second.value = v;
                fit->second.initialized = true;
                return;
            }
        }
        m_env.back()[name] = {v, false, true};
    }

    Value RuntimeEvaluator::call(FunctionDeclaration* fn, const std::vector<Value>& args) {
        beginScope();
        for (size_t i = 0; i < fn->params.size() && i < args.size(); ++i) {
            m_env.back()[fn->params[i]->name] = {args[i], false, true};
        }
        bool prevReturn = m_hasReturn;
        m_hasReturn = false;
        if (fn->body)
            for (auto& stmt : fn->body->statements) {
                exec(stmt.get());
                if (m_hasReturn)
                    break;
            }
        Value ret = m_returnValue;
        endScope();
        m_hasReturn = prevReturn;
        return ret;
    }

    void RuntimeEvaluator::exec(Statement* s) {
        if (!s)
            return;
        if (auto var = dynamic_cast<VariableDeclaration*>(s)) {
            Value v;
            if (auto prim = dynamic_cast<PrimitiveType*>(var->varType.get())) {
                if (prim->name == "int")
                    v.type = Value::Type::Int;
                else if (prim->name == "bit")
                    v.type = Value::Type::Bit;
                else if (prim->name == "float")
                    v.type = Value::Type::Float;
                else if (prim->name == "string")
                    v.type = Value::Type::String;
                else if (prim->name == "qubit") {
                    v.type = Value::Type::Qubit;
                    v.qubit = allocateTrackedQubit(var->name);
                }
            }
            bool initialized = false;
            if (var->initializer) {
                v = eval(var->initializer.get());
                initialized = true;
            }
            m_env.back()[var->name] = {v, var->isTracked, initialized};
        } else if (auto block = dynamic_cast<BlockStatement*>(s)) {
            beginScope();
            for (auto& st : block->statements) {
                exec(st.get());
                if (m_hasReturn)
                    break;
            }
            endScope();
        } else if (auto exprs = dynamic_cast<ExpressionStatement*>(s)) {
            eval(exprs->expression.get());
        } else if (auto ret = dynamic_cast<ReturnStatement*>(s)) {
            if (ret->value)
                m_returnValue = eval(ret->value.get());
            m_hasReturn = true;
        } else if (auto ifs = dynamic_cast<IfStatement*>(s)) {
            Value cond = eval(ifs->condition.get());
            if (cond.intValue || cond.bitValue) {
                exec(ifs->thenBranch.get());
            } else {
                exec(ifs->elseBranch.get());
            }
        } else if (auto tern = dynamic_cast<TernaryStatement*>(s)) {
            Value cond = eval(tern->condition.get());
            if (cond.intValue || cond.bitValue) {
                exec(tern->thenBranch.get());
            } else {
                exec(tern->elseBranch.get());
            }
        } else if (auto fors = dynamic_cast<ForStatement*>(s)) {
            beginScope(); 
            if (fors->initializer) exec(fors->initializer.get());
            while (true) {
                Value c{Value::Type::Bit};
                if (fors->condition)
                    c = eval(fors->condition.get());
                if (!(c.intValue || c.bitValue))
                    break;
                exec(fors->body.get());
                if (m_hasReturn)
                    break;
                if (fors->increment)
                    eval(fors->increment.get());
            }
            endScope();
        } else if (auto whiles = dynamic_cast<WhileStatement*>(s)) {
            while (true) {
                Value c{Value::Type::Bit};
                if (whiles->condition)
                    c = eval(whiles->condition.get());
                if (!(c.intValue || c.bitValue))
                    break;
                exec(whiles->body.get());
                if (m_hasReturn)
                    break;
            }
        } else if (auto echo = dynamic_cast<EchoStatement*>(s)) {
            Value v = eval(echo->value.get());
            if (m_echoEnabled)
                std::cout << valueToString(v) << std::endl;
        } else if (auto reset = dynamic_cast<ResetStatement*>(s)) {
            // ignore
        } else if (auto meas = dynamic_cast<MeasureStatement*>(s)) {
            Value q = eval(meas->qubit.get());
            m_sim.measure(q.qubit);
            markMeasured(q.qubit);
        } else if (auto assignStmt = dynamic_cast<AssignmentStatement*>(s)) {
            Value val = eval(assignStmt->value.get());
            assign(assignStmt->name, val);
        }
    }

    Value RuntimeEvaluator::eval(Expression* e) {
        if (!e)
            return {};
        if (auto lit = dynamic_cast<LiteralExpression*>(e)) {
            Value v;
            if (lit->literalType == "bit") {
                v.type = Value::Type::Bit;
                v.bitValue = std::stoi(lit->value);
            } else if (lit->literalType == "float") {
                v.type = Value::Type::Float;
                v.floatValue = std::stof(lit->value);
            } else if (lit->literalType == "string") {
                v.type = Value::Type::String;
                if (lit->value.size() >= 2)
                    v.stringValue = lit->value.substr(1, lit->value.size() - 2);
                else
                    v.stringValue = "";
            } else {
                v.type = Value::Type::Int;
                v.intValue = std::stoi(lit->value);
            }
            return v;
        } else if (auto var = dynamic_cast<VariableExpression*>(e)) {
            return lookup(var->name);
        } else if (auto bin = dynamic_cast<BinaryExpression*>(e)) {
            Value l = eval(bin->left.get());
            Value r = eval(bin->right.get());
            auto lInt = l.type == Value::Type::Bit ? l.bitValue : l.intValue;
            auto rInt = r.type == Value::Type::Bit ? r.bitValue : r.intValue;
            double lNum = l.type == Value::Type::Float ? l.floatValue : static_cast<double>(lInt);
            double rNum = r.type == Value::Type::Float ? r.floatValue : static_cast<double>(rInt);
            if (bin->op == "+") {
                if (l.type == Value::Type::String || r.type == Value::Type::String) {
                    return {Value::Type::String, 0, 0.0, 0, valueToString(l) + valueToString(r)};
                }
                if (l.type == Value::Type::Float || r.type == Value::Type::Float) {
                    return {Value::Type::Float, 0, lNum + rNum};
                }
                return {Value::Type::Int, lInt + rInt};
            }
            if (bin->op == "-") {
                if (l.type == Value::Type::Float || r.type == Value::Type::Float)
                    return {Value::Type::Float, 0, lNum - rNum};
                return {Value::Type::Int, lInt - rInt};
            }
            if (bin->op == "*") {
                if (l.type == Value::Type::Float || r.type == Value::Type::Float)
                    return {Value::Type::Float, 0, lNum * rNum};
                return {Value::Type::Int, lInt * rInt};
            }
            if (bin->op == "/") {
                if (rNum == 0) {
                    throw std::runtime_error("Division by zero in expression evaluation");
                }
                if (l.type == Value::Type::Float || r.type == Value::Type::Float) {
                    return {Value::Type::Float, 0, lNum / rNum};
                }
                return {Value::Type::Int, static_cast<int>(lNum / rNum)};
            }
            if (bin->op == "%") {
                if (rInt == 0) {
                    throw std::runtime_error("Modulo by zero in expression evaluation");
                }
                return {Value::Type::Int, lInt % rInt};
            }

            if (bin->op == ">") {
                if (l.type == Value::Type::Float || r.type == Value::Type::Float)
                    return {Value::Type::Bit, 0, 0.0, lNum > rNum};
                return {Value::Type::Bit, 0, 0.0, lInt > rInt};
            }
            if (bin->op == "<") {
                if (l.type == Value::Type::Float || r.type == Value::Type::Float)
                    return {Value::Type::Bit, 0, 0.0, lNum < rNum};
                return {Value::Type::Bit, 0, 0.0, lInt < rInt};
            }
            if (bin->op == ">=") {
                if (l.type == Value::Type::Float || r.type == Value::Type::Float)
                    return {Value::Type::Bit, 0, 0.0, lNum >= rNum};
                return {Value::Type::Bit, 0, 0.0, lInt >= rInt};
            }
            if (bin->op == "<=") {
                if (l.type == Value::Type::Float || r.type == Value::Type::Float)
                    return {Value::Type::Bit, 0, 0.0, lNum <= rNum};
                return {Value::Type::Bit, 0, 0.0, lInt <= rInt};
            }
            if (bin->op == "==") {
                if (l.type == Value::Type::Float || r.type == Value::Type::Float)
                    return {Value::Type::Bit, 0, 0.0, lNum == rNum};
                return {Value::Type::Bit, 0, 0.0, lInt == rInt};
            }
            if (bin->op == "!=") {
                if (l.type == Value::Type::Float || r.type == Value::Type::Float)
                    return {Value::Type::Bit, 0, 0.0, lNum != rNum};
                return {Value::Type::Bit, 0, 0.0, lInt != rInt};
            }
        } else if (auto unary = dynamic_cast<UnaryExpression*>(e)) {
            Value r = eval(unary->right.get());
            if (unary->op == "-") {
                if (r.type == Value::Type::Float)
                    return {Value::Type::Float, 0, -r.floatValue};
                return {Value::Type::Int, -r.intValue};
            }
            return r;
        } else if (auto post = dynamic_cast<PostfixExpression*>(e)) {
            if (auto var = dynamic_cast<VariableExpression*>(post->left.get())) {
                Value current = lookup(var->name);
                Value updated = current;
                if (post->op == "++") {
                    if (current.type == Value::Type::Float)
                        updated.floatValue += 1.0;
                    else if (current.type == Value::Type::Int)
                        updated.intValue += 1;
                } else if (post->op == "--") {
                    if (current.type == Value::Type::Float)
                        updated.floatValue -= 1.0;
                    else if (current.type == Value::Type::Int)
                        updated.intValue -= 1;
                }
                assign(var->name, updated);
                return current;
            }
            return {};
        } else if (auto callExpr = dynamic_cast<CallExpression*>(e)) {
            if (auto var = dynamic_cast<VariableExpression*>(callExpr->callee.get())) {
                auto name = var->name;
                auto builtin = builtInGates.find(name);
                std::vector<Value> args;
                for (auto& a : callExpr->arguments) args.push_back(eval(a.get()));
                if (builtin != builtInGates.end()) {
                    if (name == "h")
                        m_sim.h(args[0].qubit);
                    else if (name == "x")
                        m_sim.x(args[0].qubit);
                    else if (name == "y")
                        m_sim.y(args[0].qubit);
                    else if (name == "z")
                        m_sim.z(args[0].qubit);
                    else if (name == "rx")
                        m_sim.rx(args[0].qubit, args[1].floatValue);
                    else if (name == "ry")
                        m_sim.ry(args[0].qubit, args[1].floatValue);
                    else if (name == "rz")
                        m_sim.rz(args[0].qubit, args[1].floatValue);
                    else if (name == "cx")
                        m_sim.cx(args[0].qubit, args[1].qubit);
                    return {};  // void
                }
                auto fit = m_functions.find(name);
                if (fit != m_functions.end()) {
                    auto res = call(fit->second, args);
                    if (fit->second->hasQuantumAnnotation && res.type == Value::Type::Bit) {
                        m_measurements[e].push_back(res.bitValue);
                    }
                    return res;
                }
            }
        } else if (auto idx = dynamic_cast<MeasureExpression*>(e)) {
            Value q = eval(idx->qubit.get());
            int bit = m_sim.measure(q.qubit);
            markMeasured(q.qubit);
            m_measurements[e].push_back(bit);
            return {Value::Type::Bit, 0, 0.0, bit};
        } else if (auto assignExpr = dynamic_cast<AssignmentExpression*>(e)) {
            Value v = eval(assignExpr->value.get());
            assign(assignExpr->name, v);
            return v;
        }
        return {};
    }

    int RuntimeEvaluator::allocateTrackedQubit(const std::string& name) {
        int idx = m_sim.allocateQubit();
        m_qubits.push_back({name, false});
        return idx;
    }

    void RuntimeEvaluator::markMeasured(int index) {
        if (index >= 0 && index < static_cast<int>(m_qubits.size()))
            m_qubits[index].measured = true;
    }

    void RuntimeEvaluator::warnUnmeasured() const {
        for (const auto& q : m_qubits) {
            if (!q.measured) {
                std::cerr << "Warning: Qubit " << q.name
                          << " was left unmeasured. No classical value will be returned.\n";
            }
        }
    }

    void RuntimeEvaluator::beginScope() { m_env.push_back({}); }

    void RuntimeEvaluator::endScope() {
        if (m_env.empty())
            return;
        for (auto& kv : m_env.back()) {
            if (kv.second.tracked) {
                std::string key =
                    kv.second.initialized ? valueToString(kv.second.value) : "__unassigned__";
                m_trackedCounts[kv.first][key]++;
            }
        }
        m_env.pop_back();
    }
}