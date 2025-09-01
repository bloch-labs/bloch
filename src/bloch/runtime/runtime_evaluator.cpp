#include "runtime_evaluator.hpp"
#include <sstream>
#include "../error/bloch_error.hpp"
#include "../semantics/built_ins.hpp"

namespace bloch {

    static std::string valueToString(const Value& v) {
        // Pretty-print a runtime value for echo and tracked summaries.
        std::ostringstream oss;
        switch (v.type) {
            case Value::Type::String:
                return v.stringValue;
            case Value::Type::Char: {
                oss << "'" << v.charValue << "'";
                return oss.str();
            }
            case Value::Type::Float:
                oss << v.floatValue;
                return oss.str();
            case Value::Type::Bit:
                return std::to_string(v.bitValue);
            case Value::Type::Int:
                return std::to_string(v.intValue);
            case Value::Type::BitArray:
                oss << "{";
                for (size_t i = 0; i < v.bitArray.size(); ++i) {
                    if (i)
                        oss << ", ";
                    oss << v.bitArray[i];
                }
                oss << "}";
                return oss.str();
            case Value::Type::IntArray:
                oss << "{";
                for (size_t i = 0; i < v.intArray.size(); ++i) {
                    if (i)
                        oss << ", ";
                    oss << v.intArray[i];
                }
                oss << "}";
                return oss.str();
            case Value::Type::FloatArray:
                oss << "{";
                for (size_t i = 0; i < v.floatArray.size(); ++i) {
                    if (i)
                        oss << ", ";
                    oss << v.floatArray[i];
                }
                oss << "}";
                return oss.str();
            case Value::Type::StringArray:
                oss << "{";
                for (size_t i = 0; i < v.stringArray.size(); ++i) {
                    if (i)
                        oss << ", ";
                    oss << v.stringArray[i];
                }
                oss << "}";
                return oss.str();
            case Value::Type::CharArray:
                oss << "{";
                for (size_t i = 0; i < v.charArray.size(); ++i) {
                    if (i)
                        oss << ", ";
                    oss << "'" << v.charArray[i] << "'";
                }
                oss << "}";
                return oss.str();
            default:
                return "";
        }
    }

    void RuntimeEvaluator::execute(Program& program) {
        m_echoBuffer.clear();
        for (auto& fn : program.functions) {
            m_functions[fn->name] = fn.get();
        }
        auto it = m_functions.find("main");
        if (it != m_functions.end()) {
            call(it->second, {});
        }
        // Ensure warnings appear before any normal echo output
        if (m_warnOnExit)
            warnUnmeasured();
        flushEchoes();
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
        // Bind parameters, run the body until a return is hit, then unwind.
        beginScope();
        for (size_t i = 0; i < fn->params.size() && i < args.size(); ++i) {
            m_env.back()[fn->params[i]->name] = {args[i], false, true};
        }
        bool prevReturn = m_hasReturn;
        m_hasReturn = false;
        if (fn->body) {
            for (auto& stmt : fn->body->statements) {
                exec(stmt.get());
                if (m_hasReturn)
                    break;
            }
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
                else if (prim->name == "char")
                    v.type = Value::Type::Char;
                else if (prim->name == "qubit") {
                    v.type = Value::Type::Qubit;
                    v.qubit = allocateTrackedQubit(var->name);
                }
            } else if (auto arr = dynamic_cast<ArrayType*>(var->varType.get())) {
                if (auto elem = dynamic_cast<PrimitiveType*>(arr->elementType.get())) {
                    if (elem->name == "bit")
                        v.type = Value::Type::BitArray;
                    else if (elem->name == "int")
                        v.type = Value::Type::IntArray;
                    else if (elem->name == "float")
                        v.type = Value::Type::FloatArray;
                    else if (elem->name == "string")
                        v.type = Value::Type::StringArray;
                    else if (elem->name == "char")
                        v.type = Value::Type::CharArray;
                    else if (elem->name == "qubit")
                        v.type = Value::Type::QubitArray;
                }
                // Handle fixed-size allocation (without initializer)
                if (arr->size >= 0 && !var->initializer) {
                    int n = arr->size;
                    if (v.type == Value::Type::BitArray)
                        v.bitArray.assign(n, 0);
                    else if (v.type == Value::Type::IntArray)
                        v.intArray.assign(n, 0);
                    else if (v.type == Value::Type::FloatArray)
                        v.floatArray.assign(n, 0.0);
                    else if (v.type == Value::Type::StringArray)
                        v.stringArray.assign(n, "");
                    else if (v.type == Value::Type::CharArray)
                        v.charArray.assign(n, '\0');
                    else if (v.type == Value::Type::QubitArray) {
                        v.qubitArray.resize(n);
                        for (int i = 0; i < n; ++i)
                            v.qubitArray[i] = allocateTrackedQubit(var->name);
                    }
                }
            }
            bool initialized = false;
            if (var->initializer) {
                // Special case of array literal initialisation for typed arrays
                if (auto arrType = dynamic_cast<ArrayType*>(var->varType.get())) {
                    if (auto elem = dynamic_cast<PrimitiveType*>(arrType->elementType.get())) {
                        if (elem->name == "qubit") {
                            throw BlochError(var->line, var->column,
                                             "qubit[] cannot be initialised");
                        }
                        if (auto arrLit =
                                dynamic_cast<ArrayLiteralExpression*>(var->initializer.get())) {
                            // If size is specified, enforce it
                            if (arrType->size >= 0 &&
                                static_cast<int>(arrLit->elements.size()) != arrType->size) {
                                throw BlochError(
                                    var->line, var->column,
                                    "Array initializer length does not match declared size");
                            }
                            if (elem->name == "bit") {
                                v.type = Value::Type::BitArray;
                                v.bitArray.clear();
                                for (auto& el : arrLit->elements) {
                                    Value ev = eval(el.get());
                                    if (ev.type != Value::Type::Bit)
                                        throw BlochError(el->line, el->column,
                                                         "bit[] initialiser expects bit elements");
                                    v.bitArray.push_back(ev.bitValue ? 1 : 0);
                                }
                            } else if (elem->name == "int") {
                                v.type = Value::Type::IntArray;
                                v.intArray.clear();
                                for (auto& el : arrLit->elements) {
                                    Value ev = eval(el.get());
                                    int val = 0;
                                    if (ev.type == Value::Type::Int)
                                        val = ev.intValue;
                                    else if (ev.type == Value::Type::Bit)
                                        val = ev.bitValue;
                                    else if (ev.type == Value::Type::Float)
                                        val = static_cast<int>(ev.floatValue);
                                    else
                                        throw BlochError(
                                            el->line, el->column,
                                            "int[] initialiser expects integer elements");
                                    v.intArray.push_back(val);
                                }
                            } else if (elem->name == "float") {
                                v.type = Value::Type::FloatArray;
                                v.floatArray.clear();
                                for (auto& el : arrLit->elements) {
                                    Value ev = eval(el.get());
                                    double val = 0.0;
                                    if (ev.type == Value::Type::Float)
                                        val = ev.floatValue;
                                    else if (ev.type == Value::Type::Int)
                                        val = static_cast<double>(ev.intValue);
                                    else if (ev.type == Value::Type::Bit)
                                        val = static_cast<double>(ev.bitValue);
                                    else
                                        throw BlochError(
                                            el->line, el->column,
                                            "float[] initialiser expects float elements");
                                    v.floatArray.push_back(val);
                                }
                            } else if (elem->name == "string") {
                                v.type = Value::Type::StringArray;
                                v.stringArray.clear();
                                for (auto& el : arrLit->elements) {
                                    Value ev = eval(el.get());
                                    if (ev.type != Value::Type::String)
                                        throw BlochError(
                                            el->line, el->column,
                                            "string[] initialiser expects string elements");
                                    v.stringArray.push_back(ev.stringValue);
                                }
                            } else if (elem->name == "char") {
                                v.type = Value::Type::CharArray;
                                v.charArray.clear();
                                for (auto& el : arrLit->elements) {
                                    Value ev = eval(el.get());
                                    if (ev.type == Value::Type::Char)
                                        v.charArray.push_back(ev.charValue);
                                    else
                                        throw BlochError(
                                            el->line, el->column,
                                            "char[] initialiser expects char elements");
                                }
                            }
                            initialized = true;
                        } else {
                            // Fallback: evaluate as expression
                            v = eval(var->initializer.get());
                            initialized = true;
                        }
                    }
                } else {
                    v = eval(var->initializer.get());
                    initialized = true;
                }
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
            if (fors->initializer)
                exec(fors->initializer.get());
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
                m_echoBuffer.push_back(valueToString(v));
        } else if (auto reset = dynamic_cast<ResetStatement*>(s)) {
            Value q = eval(reset->target.get());
            m_sim.reset(q.qubit);
            unmarkMeasured(q.qubit);
        } else if (auto meas = dynamic_cast<MeasureStatement*>(s)) {
            Value q = eval(meas->qubit.get());
            int bit = m_sim.measure(q.qubit);
            markMeasured(q.qubit);
            if (q.qubit >= 0 && q.qubit < static_cast<int>(m_lastMeasurement.size()))
                m_lastMeasurement[q.qubit] = bit;
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
            } else if (lit->literalType == "char") {
                v.type = Value::Type::Char;
                if (lit->value.size() >= 3)
                    v.charValue = lit->value[1];
                else
                    v.charValue = '\0';
            } else {
                v.type = Value::Type::Int;
                v.intValue = std::stoi(lit->value);
            }
            return v;
        } else if (auto var = dynamic_cast<VariableExpression*>(e)) {
            return lookup(var->name);
        } else if (auto arr = dynamic_cast<ArrayLiteralExpression*>(e)) {
            // Infer array type from first element (if present)
            Value v;
            if (arr->elements.empty()) {
                v.type = Value::Type::IntArray;
                return v;  // empty, default as int[] when untyped
            }
            Value first = eval(arr->elements[0].get());
            switch (first.type) {
                case Value::Type::Bit:
                    v.type = Value::Type::BitArray;
                    for (auto& el : arr->elements) {
                        Value ev = eval(el.get());
                        if (ev.type != Value::Type::Bit)
                            throw BlochError(el->line, el->column,
                                             "Inconsistent element types in array literal");
                        v.bitArray.push_back(ev.bitValue ? 1 : 0);
                    }
                    break;
                case Value::Type::Int:
                    v.type = Value::Type::IntArray;
                    for (auto& el : arr->elements) {
                        Value ev = eval(el.get());
                        if (ev.type != Value::Type::Int && ev.type != Value::Type::Bit)
                            throw BlochError(el->line, el->column,
                                             "Inconsistent element types in array literal");
                        v.intArray.push_back(ev.type == Value::Type::Int ? ev.intValue
                                                                         : ev.bitValue);
                    }
                    break;
                case Value::Type::Float:
                    v.type = Value::Type::FloatArray;
                    for (auto& el : arr->elements) {
                        Value ev = eval(el.get());
                        if (ev.type != Value::Type::Float && ev.type != Value::Type::Int &&
                            ev.type != Value::Type::Bit)
                            throw BlochError(el->line, el->column,
                                             "Inconsistent element types in array literal");
                        double val =
                            (ev.type == Value::Type::Float)
                                ? ev.floatValue
                                : static_cast<double>(ev.type == Value::Type::Int ? ev.intValue
                                                                                  : ev.bitValue);
                        v.floatArray.push_back(val);
                    }
                    break;
                case Value::Type::String:
                    v.type = Value::Type::StringArray;
                    for (auto& el : arr->elements) {
                        Value ev = eval(el.get());
                        if (ev.type != Value::Type::String)
                            throw BlochError(el->line, el->column,
                                             "Inconsistent element types in array literal");
                        v.stringArray.push_back(ev.stringValue);
                    }
                    break;
                case Value::Type::Char:
                    v.type = Value::Type::CharArray;
                    for (auto& el : arr->elements) {
                        Value ev = eval(el.get());
                        if (ev.type != Value::Type::Char)
                            throw BlochError(el->line, el->column,
                                             "Inconsistent element types in array literal");
                        v.charArray.push_back(ev.charValue);
                    }
                    break;
                default:
                    throw BlochError(arr->line, arr->column, "Unsupported array literal type");
            }
            return v;
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
                    throw BlochError(bin->line, bin->column,
                                     "Division by zero in expression evaluation");
                }
                if (l.type == Value::Type::Float || r.type == Value::Type::Float) {
                    return {Value::Type::Float, 0, lNum / rNum};
                }
                return {Value::Type::Int, static_cast<int>(lNum / rNum)};
            }
            if (bin->op == "%") {
                if (rInt == 0) {
                    throw BlochError(bin->line, bin->column,
                                     "Modulo by zero in expression evaluation");
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
            if (bin->op == "&&") {
                bool lb = l.type == Value::Type::Float ? lNum != 0.0 : lInt != 0;
                bool rb = r.type == Value::Type::Float ? rNum != 0.0 : rInt != 0;
                return {Value::Type::Bit, 0, 0.0, lb && rb};
            }
            if (bin->op == "||") {
                bool lb = l.type == Value::Type::Float ? lNum != 0.0 : lInt != 0;
                bool rb = r.type == Value::Type::Float ? rNum != 0.0 : rInt != 0;
                return {Value::Type::Bit, 0, 0.0, lb || rb};
            }
            if (bin->op == "&") {
                if (l.type == Value::Type::Bit && r.type == Value::Type::Bit)
                    return {Value::Type::Bit, 0, 0.0, l.bitValue & r.bitValue};
                if (l.type == Value::Type::BitArray && r.type == Value::Type::BitArray) {
                    if (l.bitArray.size() != r.bitArray.size()) {
                        throw BlochError(bin->line, bin->column,
                                         "Bit arrays must be same length for '&'");
                    }
                    Value v;
                    v.type = Value::Type::BitArray;
                    v.bitArray.resize(l.bitArray.size());
                    for (size_t i = 0; i < l.bitArray.size(); ++i)
                        v.bitArray[i] = l.bitArray[i] & r.bitArray[i];
                    return v;
                }
                if (l.type == Value::Type::BitArray && r.type == Value::Type::Bit) {
                    Value v;
                    v.type = Value::Type::BitArray;
                    v.bitArray.resize(l.bitArray.size());
                    for (size_t i = 0; i < l.bitArray.size(); ++i)
                        v.bitArray[i] = l.bitArray[i] & r.bitValue;
                    return v;
                }
                if (l.type == Value::Type::Bit && r.type == Value::Type::BitArray) {
                    Value v;
                    v.type = Value::Type::BitArray;
                    v.bitArray.resize(r.bitArray.size());
                    for (size_t i = 0; i < r.bitArray.size(); ++i)
                        v.bitArray[i] = l.bitValue & r.bitArray[i];
                    return v;
                }
                throw BlochError(bin->line, bin->column,
                                 "Bitwise '&' requires bit or bit[] operands");
            }
            if (bin->op == "|") {
                if (l.type == Value::Type::Bit && r.type == Value::Type::Bit)
                    return {Value::Type::Bit, 0, 0.0, l.bitValue | r.bitValue};
                if (l.type == Value::Type::BitArray && r.type == Value::Type::BitArray) {
                    if (l.bitArray.size() != r.bitArray.size()) {
                        throw BlochError(bin->line, bin->column,
                                         "Bit arrays must be same length for '|'");
                    }
                    Value v;
                    v.type = Value::Type::BitArray;
                    v.bitArray.resize(l.bitArray.size());
                    for (size_t i = 0; i < l.bitArray.size(); ++i)
                        v.bitArray[i] = l.bitArray[i] | r.bitArray[i];
                    return v;
                }
                if (l.type == Value::Type::BitArray && r.type == Value::Type::Bit) {
                    Value v;
                    v.type = Value::Type::BitArray;
                    v.bitArray.resize(l.bitArray.size());
                    for (size_t i = 0; i < l.bitArray.size(); ++i)
                        v.bitArray[i] = l.bitArray[i] | r.bitValue;
                    return v;
                }
                if (l.type == Value::Type::Bit && r.type == Value::Type::BitArray) {
                    Value v;
                    v.type = Value::Type::BitArray;
                    v.bitArray.resize(r.bitArray.size());
                    for (size_t i = 0; i < r.bitArray.size(); ++i)
                        v.bitArray[i] = l.bitValue | r.bitArray[i];
                    return v;
                }
                throw BlochError(bin->line, bin->column,
                                 "Bitwise '|' requires bit or bit[] operands");
            }
            if (bin->op == "^") {
                if (l.type == Value::Type::Bit && r.type == Value::Type::Bit)
                    return {Value::Type::Bit, 0, 0.0, l.bitValue ^ r.bitValue};
                if (l.type == Value::Type::BitArray && r.type == Value::Type::BitArray) {
                    if (l.bitArray.size() != r.bitArray.size()) {
                        throw BlochError(bin->line, bin->column,
                                         "Bit arrays must be same length for '^'");
                    }
                    Value v;
                    v.type = Value::Type::BitArray;
                    v.bitArray.resize(l.bitArray.size());
                    for (size_t i = 0; i < l.bitArray.size(); ++i)
                        v.bitArray[i] = l.bitArray[i] ^ r.bitArray[i];
                    return v;
                }
                if (l.type == Value::Type::BitArray && r.type == Value::Type::Bit) {
                    Value v;
                    v.type = Value::Type::BitArray;
                    v.bitArray.resize(l.bitArray.size());
                    for (size_t i = 0; i < l.bitArray.size(); ++i)
                        v.bitArray[i] = l.bitArray[i] ^ r.bitValue;
                    return v;
                }
                if (l.type == Value::Type::Bit && r.type == Value::Type::BitArray) {
                    Value v;
                    v.type = Value::Type::BitArray;
                    v.bitArray.resize(r.bitArray.size());
                    for (size_t i = 0; i < r.bitArray.size(); ++i)
                        v.bitArray[i] = l.bitValue ^ r.bitArray[i];
                    return v;
                }
                throw BlochError(bin->line, bin->column,
                                 "Bitwise '^' requires bit or bit[] operands");
            }
        } else if (auto unary = dynamic_cast<UnaryExpression*>(e)) {
            Value r = eval(unary->right.get());
            if (unary->op == "-") {
                if (r.type == Value::Type::Float)
                    return {Value::Type::Float, 0, -r.floatValue};
                return {Value::Type::Int, -r.intValue};
            }
            if (unary->op == "!") {
                if (r.type == Value::Type::BitArray) {
                    throw BlochError(bin->line, bin->column, "Logical '!' unsupported for bit[]");
                }
                bool rb = r.type == Value::Type::Float
                              ? r.floatValue != 0.0
                              : (r.type == Value::Type::Bit ? r.bitValue != 0 : r.intValue != 0);
                return {Value::Type::Bit, 0, 0.0, !rb};
            }
            if (unary->op == "~") {
                if (r.type == Value::Type::Bit)
                    return {Value::Type::Bit, 0, 0.0, r.bitValue ? 0 : 1};
                if (r.type == Value::Type::BitArray) {
                    Value v;
                    v.type = Value::Type::BitArray;
                    v.bitArray.resize(r.bitArray.size());
                    for (size_t i = 0; i < r.bitArray.size(); ++i)
                        v.bitArray[i] = r.bitArray[i] ? 0 : 1;
                    return v;
                }
                throw BlochError(bin->line, bin->column,
                                 "Bitwise '~' requires bit or bit[] operand");
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
                    // Map built-ins directly to simulator operations.
                    // TODO: In the noisy simulator this logic will have to remain the same
                    // so we will need the same basic quantum operations
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
            if (q.qubit >= 0 && q.qubit < static_cast<int>(m_lastMeasurement.size()))
                m_lastMeasurement[q.qubit] = bit;
            m_measurements[e].push_back(bit);
            return {Value::Type::Bit, 0, 0.0, bit};
        } else if (auto indexExpr = dynamic_cast<IndexExpression*>(e)) {
            Value coll = eval(indexExpr->collection.get());
            Value idxv = eval(indexExpr->index.get());
            int idxi = 0;
            if (idxv.type == Value::Type::Int)
                idxi = idxv.intValue;
            else if (idxv.type == Value::Type::Bit)
                idxi = idxv.bitValue;
            else if (idxv.type == Value::Type::Float)
                idxi = static_cast<int>(idxv.floatValue);
            else
                throw BlochError(indexExpr->line, indexExpr->column, "Index must be numeric");
            if (idxi < 0)
                throw BlochError(indexExpr->line, indexExpr->column, "Index out of bounds");
            switch (coll.type) {
                case Value::Type::BitArray:
                    if (idxi >= static_cast<int>(coll.bitArray.size()))
                        throw BlochError(indexExpr->line, indexExpr->column, "Index out of bounds");
                    return {Value::Type::Bit, 0, 0.0, coll.bitArray[idxi]};
                case Value::Type::IntArray:
                    if (idxi >= static_cast<int>(coll.intArray.size()))
                        throw BlochError(indexExpr->line, indexExpr->column, "Index out of bounds");
                    return {Value::Type::Int, coll.intArray[idxi]};
                case Value::Type::FloatArray:
                    if (idxi >= static_cast<int>(coll.floatArray.size()))
                        throw BlochError(indexExpr->line, indexExpr->column, "Index out of bounds");
                    return {Value::Type::Float, 0, coll.floatArray[idxi]};
                case Value::Type::StringArray:
                    if (idxi >= static_cast<int>(coll.stringArray.size()))
                        throw BlochError(indexExpr->line, indexExpr->column, "Index out of bounds");
                    return {Value::Type::String, 0, 0.0, 0, coll.stringArray[idxi]};
                case Value::Type::CharArray:
                    if (idxi >= static_cast<int>(coll.charArray.size()))
                        throw BlochError(indexExpr->line, indexExpr->column, "Index out of bounds");
                    {
                        Value v;
                        v.type = Value::Type::Char;
                        v.charValue = coll.charArray[idxi];
                        return v;
                    }
                case Value::Type::QubitArray: {
                    if (idxi >= static_cast<int>(coll.qubitArray.size()))
                        throw BlochError(indexExpr->line, indexExpr->column, "Index out of bounds");
                    Value v;
                    v.type = Value::Type::Qubit;
                    v.qubit = coll.qubitArray[idxi];
                    return v;
                }
                default:
                    throw BlochError(indexExpr->line, indexExpr->column,
                                     "Indexing requires an array value");
            }
        } else if (auto assignExpr = dynamic_cast<AssignmentExpression*>(e)) {
            Value v = eval(assignExpr->value.get());
            assign(assignExpr->name, v);
            return v;
        } else if (auto aassign = dynamic_cast<ArrayAssignmentExpression*>(e)) {
            // Only support assigning into variable arrays for 1.0.0
            auto* var = dynamic_cast<VariableExpression*>(aassign->collection.get());
            if (!var)
                throw BlochError(aassign->line, aassign->column,
                                 "Assignment target must be a variable array");
            Value arr = lookup(var->name);
            Value idxv = eval(aassign->index.get());
            int i = 0;
            if (idxv.type == Value::Type::Int)
                i = idxv.intValue;
            else if (idxv.type == Value::Type::Bit)
                i = idxv.bitValue;
            else if (idxv.type == Value::Type::Float)
                i = static_cast<int>(idxv.floatValue);
            else
                throw BlochError(aassign->line, aassign->column, "Index must be numeric");
            if (i < 0)
                throw BlochError(aassign->line, aassign->column, "Index out of bounds");
            Value rhs = eval(aassign->value.get());
            switch (arr.type) {
                case Value::Type::IntArray:
                    if (i >= static_cast<int>(arr.intArray.size()))
                        throw BlochError(aassign->line, aassign->column, "Index out of bounds");
                    if (rhs.type == Value::Type::Int)
                        arr.intArray[i] = rhs.intValue;
                    else if (rhs.type == Value::Type::Bit)
                        arr.intArray[i] = rhs.bitValue;
                    else if (rhs.type == Value::Type::Float)
                        arr.intArray[i] = static_cast<int>(rhs.floatValue);
                    else
                        throw BlochError(aassign->line, aassign->column,
                                         "Type mismatch for int[] assignment");
                    break;
                case Value::Type::FloatArray:
                    if (i >= static_cast<int>(arr.floatArray.size()))
                        throw BlochError(aassign->line, aassign->column, "Index out of bounds");
                    if (rhs.type == Value::Type::Float)
                        arr.floatArray[i] = rhs.floatValue;
                    else if (rhs.type == Value::Type::Int)
                        arr.floatArray[i] = static_cast<double>(rhs.intValue);
                    else if (rhs.type == Value::Type::Bit)
                        arr.floatArray[i] = static_cast<double>(rhs.bitValue);
                    else
                        throw BlochError(aassign->line, aassign->column,
                                         "Type mismatch for float[] assignment");
                    break;
                case Value::Type::BitArray:
                    if (i >= static_cast<int>(arr.bitArray.size()))
                        throw BlochError(aassign->line, aassign->column, "Index out of bounds");
                    if (rhs.type == Value::Type::Bit)
                        arr.bitArray[i] = rhs.bitValue ? 1 : 0;
                    else if (rhs.type == Value::Type::Int)
                        arr.bitArray[i] = (rhs.intValue != 0) ? 1 : 0;
                    else
                        throw BlochError(aassign->line, aassign->column,
                                         "Type mismatch for bit[] assignment");
                    break;
                case Value::Type::StringArray:
                    if (i >= static_cast<int>(arr.stringArray.size()))
                        throw BlochError(aassign->line, aassign->column, "Index out of bounds");
                    if (rhs.type == Value::Type::String)
                        arr.stringArray[i] = rhs.stringValue;
                    else
                        throw BlochError(aassign->line, aassign->column,
                                         "Type mismatch for string[] assignment");
                    break;
                case Value::Type::CharArray:
                    if (i >= static_cast<int>(arr.charArray.size()))
                        throw BlochError(aassign->line, aassign->column, "Index out of bounds");
                    if (rhs.type == Value::Type::Char)
                        arr.charArray[i] = rhs.charValue;
                    else
                        throw BlochError(aassign->line, aassign->column,
                                         "Type mismatch for char[] assignment");
                    break;
                default:
                    throw BlochError(aassign->line, aassign->column,
                                     "Assignment into this array type is unsupported");
            }
            assign(var->name, arr);
            return arr;
        }
        return {};
    }

    int RuntimeEvaluator::allocateTrackedQubit(const std::string& name) {
        int idx = m_sim.allocateQubit();
        m_qubits.push_back({name, false});
        if (idx >= static_cast<int>(m_lastMeasurement.size()))
            m_lastMeasurement.resize(idx + 1, -1);
        return idx;
    }

    void RuntimeEvaluator::markMeasured(int index) {
        if (index >= 0 && index < static_cast<int>(m_qubits.size()))
            m_qubits[index].measured = true;
    }

    void RuntimeEvaluator::unmarkMeasured(int index) {
        if (index >= 0 && index < static_cast<int>(m_qubits.size()))
            m_qubits[index].measured = false;
        if (index >= 0 && index < static_cast<int>(m_lastMeasurement.size()))
            m_lastMeasurement[index] = -1;
    }

    void RuntimeEvaluator::warnUnmeasured() const {
        for (const auto& q : m_qubits) {
            if (!q.measured) {
                blochWarning(0, 0,
                             "Qubit " + q.name +
                                 " was left unmeasured. No classical value will be returned.");
            }
        }
    }

    void RuntimeEvaluator::beginScope() { m_env.push_back({}); }

    void RuntimeEvaluator::endScope() {
        if (m_env.empty())
            return;
        for (auto& kv : m_env.back()) {
            if (!kv.second.tracked)
                continue;
            const auto& name = kv.first;
            const auto& entry = kv.second;
            const auto& v = entry.value;
            if (v.type == Value::Type::Qubit) {
                int q = v.qubit;
                std::string outcome = "?";
                if (q >= 0 && q < static_cast<int>(m_lastMeasurement.size()) &&
                    m_lastMeasurement[q] != -1) {
                    outcome = m_lastMeasurement[q] ? "1" : "0";
                }
                std::string key = std::string("qubit ") + name;
                m_trackedCounts[key][outcome]++;
            } else if (v.type == Value::Type::QubitArray) {
                bool allMeasured = true;
                std::string bits;
                for (int q : v.qubitArray) {
                    if (!(q >= 0 && q < static_cast<int>(m_lastMeasurement.size()) &&
                          m_lastMeasurement[q] != -1)) {
                        allMeasured = false;
                        break;
                    }
                }
                std::string outcome;
                if (!allMeasured) {
                    outcome = "?";
                } else {
                    for (int q : v.qubitArray) bits.push_back(m_lastMeasurement[q] ? '1' : '0');
                    outcome = bits;
                }
                std::string key = std::string("qubit[] ") + name;
                m_trackedCounts[key][outcome]++;
            }
        }
        m_env.pop_back();
    }

    void RuntimeEvaluator::flushEchoes() {
        for (const auto& line : m_echoBuffer) {
            std::cout << line << std::endl;
        }
        m_echoBuffer.clear();
    }
}
