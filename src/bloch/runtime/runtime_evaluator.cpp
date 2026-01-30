// Copyright 2026 Akshay Pal (https://bloch-labs.com)
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

#include "bloch/runtime/runtime_evaluator.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <functional>
#include <iomanip>
#include <optional>
#include <sstream>
#include <unordered_map>
#include <utility>

#include "bloch/core/semantics/built_ins.hpp"
#include "bloch/support/error/bloch_error.hpp"

namespace bloch::runtime {

    using core::builtInGates;
    using support::BlochError;
    using support::blochWarning;
    using support::ErrorCategory;

    namespace {
        std::string typeKey(const RuntimeTypeInfo& t);
        std::unique_ptr<Type> runtimeTypeToAst(const RuntimeTypeInfo& rt);
        RuntimeTypeInfo typeInfoFromValue(const Value& v);
    }

    static constexpr bool kTraceConstructors = false;

    static std::pair<RuntimeField*, RuntimeClass*> findStaticFieldWithOwner(
        RuntimeClass* cls, const std::string& name) {
        RuntimeClass* cur = cls;
        while (cur) {
            auto fit = cur->staticFieldIndex.find(name);
            if (fit != cur->staticFieldIndex.end())
                return {&cur->staticFields[fit->second], cur};
            cur = cur->base;
        }
        return {nullptr, nullptr};
    }

    static bool isNullReference(const Value& v) {
        return v.type == Value::Type::Object && !v.objectValue;
    }

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
            case Value::Type::Float: {
                // Show a trailing .0 for whole-number floats
                if (std::isfinite(v.floatValue) && std::floor(v.floatValue) == v.floatValue) {
                    oss.setf(std::ios::fixed);
                    oss << std::setprecision(1) << v.floatValue;
                } else {
                    oss << v.floatValue;
                }
                return oss.str();
            }
            case Value::Type::Bit:
                return std::to_string(v.bitValue);
            case Value::Type::Boolean:
                return v.boolValue ? "true" : "false";
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
            case Value::Type::BooleanArray:
                oss << "{";
                for (size_t i = 0; i < v.boolArray.size(); ++i) {
                    if (i)
                        oss << ", ";
                    oss << (v.boolArray[i] ? "true" : "false");
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
            case Value::Type::Object:
                if (isNullReference(v))
                    return "null";
                if (v.objectValue && v.objectValue->cls)
                    return "<" + v.objectValue->cls->name + " object>";
                return "<object>";
            case Value::Type::ObjectArray:
                oss << "{";
                for (size_t i = 0; i < v.objectArray.size(); ++i) {
                    if (i)
                        oss << ", ";
                    auto& obj = v.objectArray[i];
                    if (obj && obj->cls)
                        oss << "<" << obj->cls->name << ">";
                    else
                        oss << "null";
                }
                oss << "}";
                return oss.str();
            case Value::Type::ClassRef:
                return "<class " + v.className + ">";
            default:
                return "";
        }
    }

    RuntimeTypeInfo RuntimeEvaluator::typeInfoFromAst(
        Type* type, const std::unordered_map<std::string, RuntimeTypeInfo>& subst) const {
        RuntimeTypeInfo info;
        if (!type)
            return info;
        if (auto prim = dynamic_cast<PrimitiveType*>(type)) {
            if (prim->name == "int")
                info.kind = Value::Type::Int;
            else if (prim->name == "float")
                info.kind = Value::Type::Float;
            else if (prim->name == "bit")
                info.kind = Value::Type::Bit;
            else if (prim->name == "boolean")
                info.kind = Value::Type::Boolean;
            else if (prim->name == "string")
                info.kind = Value::Type::String;
            else if (prim->name == "char")
                info.kind = Value::Type::Char;
            else if (prim->name == "qubit")
                info.kind = Value::Type::Qubit;
        } else if (auto named = dynamic_cast<NamedType*>(type)) {
            // Type parameter substitution
            if (!named->nameParts.empty()) {
                std::string base = named->nameParts.back();
                auto it = subst.find(base);
                if (it != subst.end())
                    return it->second;
                info.kind = Value::Type::Object;
                info.className = base;
                for (auto& arg : named->typeArguments)
                    info.typeArgs.push_back(typeInfoFromAst(arg.get(), subst));
                info.className = typeKey(info);
                // Strip to base if no args (non-generic class)
                if (info.typeArgs.empty())
                    info.className = base;
            }
        } else if (auto arr = dynamic_cast<ArrayType*>(type)) {
            auto elem = typeInfoFromAst(arr->elementType.get(), subst);
            switch (elem.kind) {
                case Value::Type::Int:
                    info.kind = Value::Type::IntArray;
                    break;
                case Value::Type::Float:
                    info.kind = Value::Type::FloatArray;
                    break;
                case Value::Type::Bit:
                    info.kind = Value::Type::BitArray;
                    break;
                case Value::Type::Boolean:
                    info.kind = Value::Type::BooleanArray;
                    break;
                case Value::Type::String:
                    info.kind = Value::Type::StringArray;
                    break;
                case Value::Type::Char:
                    info.kind = Value::Type::CharArray;
                    break;
                case Value::Type::Qubit:
                    info.kind = Value::Type::QubitArray;
                    break;
                default:
                    info.kind = Value::Type::ObjectArray;
                    info.className = elem.className;
                    info.typeArgs.push_back(elem);
                    break;
            }
        } else if (dynamic_cast<VoidType*>(type)) {
            info.kind = Value::Type::Void;
        }
        return info;
    }

    namespace {
        std::string typeKey(const RuntimeTypeInfo& t) {
            if (!t.className.empty()) {
                if (t.typeArgs.empty())
                    return t.className;
                std::ostringstream oss;
                oss << t.className;
                // If className already includes args (e.g. from nesting), avoid duplication.
                if (t.className.find('<') == std::string::npos) {
                    oss << "<";
                    for (size_t i = 0; i < t.typeArgs.size(); ++i) {
                        if (i)
                            oss << ",";
                        oss << typeKey(t.typeArgs[i]);
                    }
                    oss << ">";
                }
                return oss.str();
            }
            // Primitive / array labels
            switch (t.kind) {
                case Value::Type::Int:
                    return "int";
                case Value::Type::Float:
                    return "float";
                case Value::Type::Bit:
                    return "bit";
                case Value::Type::Boolean:
                    return "boolean";
                case Value::Type::String:
                    return "string";
                case Value::Type::Char:
                    return "char";
                case Value::Type::Qubit:
                    return "qubit";
                case Value::Type::IntArray:
                    return "int[]";
                case Value::Type::FloatArray:
                    return "float[]";
                case Value::Type::BitArray:
                    return "bit[]";
                case Value::Type::BooleanArray:
                    return "boolean[]";
                case Value::Type::StringArray:
                    return "string[]";
                case Value::Type::CharArray:
                    return "char[]";
                case Value::Type::QubitArray:
                    return "qubit[]";
                case Value::Type::ObjectArray:
                    if (!t.typeArgs.empty())
                        return typeKey(t.typeArgs.front()) + "[]";
                    if (!t.className.empty())
                        return t.className + "[]";
                    return "object[]";
                default:
                    return "unknown";
            }
        }
    }  // namespace

    namespace {
        std::unique_ptr<Type> runtimeTypeToAst(const RuntimeTypeInfo& rt) {
            if (!rt.className.empty()) {
                auto nt = std::make_unique<NamedType>(std::vector<std::string>{rt.className});
                for (const auto& ta : rt.typeArgs)
                    nt->typeArguments.push_back(runtimeTypeToAst(ta));
                return nt;
            }
            switch (rt.kind) {
                case Value::Type::Int:
                    return std::make_unique<PrimitiveType>("int");
                case Value::Type::Float:
                    return std::make_unique<PrimitiveType>("float");
                case Value::Type::Bit:
                    return std::make_unique<PrimitiveType>("bit");
                case Value::Type::Boolean:
                    return std::make_unique<PrimitiveType>("boolean");
                case Value::Type::String:
                    return std::make_unique<PrimitiveType>("string");
                case Value::Type::Char:
                    return std::make_unique<PrimitiveType>("char");
                case Value::Type::Qubit:
                    return std::make_unique<PrimitiveType>("qubit");
                case Value::Type::IntArray:
                    return std::make_unique<ArrayType>(std::make_unique<PrimitiveType>("int"), -1,
                                                       nullptr);
                case Value::Type::FloatArray:
                    return std::make_unique<ArrayType>(std::make_unique<PrimitiveType>("float"), -1,
                                                       nullptr);
                case Value::Type::BitArray:
                    return std::make_unique<ArrayType>(std::make_unique<PrimitiveType>("bit"), -1,
                                                       nullptr);
                case Value::Type::BooleanArray:
                    return std::make_unique<ArrayType>(std::make_unique<PrimitiveType>("boolean"),
                                                       -1, nullptr);
                case Value::Type::StringArray:
                    return std::make_unique<ArrayType>(std::make_unique<PrimitiveType>("string"),
                                                       -1, nullptr);
                case Value::Type::CharArray:
                    return std::make_unique<ArrayType>(std::make_unique<PrimitiveType>("char"), -1,
                                                       nullptr);
                case Value::Type::ObjectArray:
                    return std::make_unique<ArrayType>(!rt.typeArgs.empty()
                                                           ? runtimeTypeToAst(rt.typeArgs.front())
                                                           : std::make_unique<PrimitiveType>("int"),
                                                       -1, nullptr);
                default:
                    return std::make_unique<VoidType>();
            }
        }

        RuntimeTypeInfo typeInfoFromValue(const Value& v) {
            RuntimeTypeInfo t;
            switch (v.type) {
                case Value::Type::Int:
                    t.kind = Value::Type::Int;
                    break;
                case Value::Type::Float:
                    t.kind = Value::Type::Float;
                    break;
                case Value::Type::Bit:
                    t.kind = Value::Type::Bit;
                    break;
                case Value::Type::Boolean:
                    t.kind = Value::Type::Boolean;
                    break;
                case Value::Type::String:
                    t.kind = Value::Type::String;
                    break;
                case Value::Type::Char:
                    t.kind = Value::Type::Char;
                    break;
                case Value::Type::Qubit:
                    t.kind = Value::Type::Qubit;
                    break;
                case Value::Type::Object:
                    t.kind = Value::Type::Object;
                    if (!v.className.empty())
                        t.className = v.className;
                    else if (v.objectValue && v.objectValue->cls)
                        t.className = v.objectValue->cls->name;
                    break;
                case Value::Type::ObjectArray:
                    t.kind = Value::Type::ObjectArray;
                    if (!v.className.empty())
                        t.className = v.className;
                    break;
                default:
                    t.kind = v.type;
                    break;
            }
            return t;
        }
    }  // namespace

    RuntimeTypeInfo RuntimeEvaluator::typeInfoFromAst(Type* type) const {
        static const std::unordered_map<std::string, RuntimeTypeInfo> emptySubst;
        return typeInfoFromAst(type, emptySubst);
    }

    Value RuntimeEvaluator::defaultValueForField(const RuntimeField& field,
                                                 const std::string& ownerLabel) {
        Value v;
        v.type = field.type.kind;
        auto makeQubitName = [&](int index) {
            std::ostringstream oss;
            oss << ownerLabel << "." << field.name;
            if (index >= 0)
                oss << "[" << index << "]";
            return oss.str();
        };
        switch (field.type.kind) {
            case Value::Type::Int:
                v.intValue = 0;
                break;
            case Value::Type::Float:
                v.floatValue = 0.0;
                break;
            case Value::Type::Bit:
                v.bitValue = 0;
                break;
            case Value::Type::Boolean:
                v.boolValue = false;
                break;
            case Value::Type::String:
                v.stringValue = "";
                break;
            case Value::Type::Char:
                v.charValue = '\0';
                break;
            case Value::Type::Qubit:
                v.qubit = allocateTrackedQubit(makeQubitName(-1));
                break;
            case Value::Type::IntArray:
                v.intArray.assign(std::max(0, field.arraySize), 0);
                break;
            case Value::Type::FloatArray:
                v.floatArray.assign(std::max(0, field.arraySize), 0.0);
                break;
            case Value::Type::BitArray:
                v.bitArray.assign(std::max(0, field.arraySize), 0);
                break;
            case Value::Type::BooleanArray:
                v.boolArray.assign(std::max(0, field.arraySize), false);
                break;
            case Value::Type::StringArray:
                v.stringArray.assign(std::max(0, field.arraySize), "");
                break;
            case Value::Type::CharArray:
                v.charArray.assign(std::max(0, field.arraySize), '\0');
                break;
            case Value::Type::QubitArray: {
                int n = std::max(0, field.arraySize);
                v.qubitArray.resize(n);
                for (int i = 0; i < n; ++i)
                    v.qubitArray[i] = allocateTrackedQubit(makeQubitName(i));
                break;
            }
            case Value::Type::ObjectArray: {
                int n = std::max(0, field.arraySize);
                v.objectArray.assign(n, {});
                break;
            }
            case Value::Type::Object:
            case Value::Type::ClassRef:
            case Value::Type::Void:
            default:
                break;
        }
        v.className = field.type.className;
        return v;
    }

    void RuntimeEvaluator::execute(Program& program) {
        if (m_executed) {
            throw BlochError(ErrorCategory::Runtime, 0, 0,
                             "RuntimeEvaluator is single-use; construct a new instance per run");
        }
        m_executed = true;
        m_functions.clear();
        m_env.clear();
        m_measurements.clear();
        m_trackedCounts.clear();
        m_echoBuffer.clear();
        m_qubits.clear();
        m_lastMeasurement.clear();
        m_returnValue = {};
        m_hasReturn = false;
        m_classTable.clear();
        m_heap.clear();
        m_currentClassCtx = nullptr;
        m_inStaticContext = false;
        m_inConstructor = false;
        m_inDestructor = false;
        m_stopGc = false;
        m_gcRequested = false;
        m_gcThreadStarted = false;
        m_allocSinceGc = 0;
        m_sim = QasmSimulator{m_collectQasmLog};
        bool hasClasses = !program.classes.empty();
        if (hasClasses) {
            buildClassTable(program);
            for (auto& kv : m_classTable) initStaticFields(kv.second.get());
            ensureGcThread();
        }
        for (auto& fn : program.functions) {
            m_functions[fn->name] = fn.get();
        }
        auto it = m_functions.find("main");
        if (it != m_functions.end()) {
            call(it->second, {});
        }
        if (m_gcThreadStarted) {
            m_stopGc = true;
            m_gcRequested = true;
            m_gcCv.notify_all();
            if (m_gcThread.joinable())
                m_gcThread.join();
        }
        runCycleCollector();
        // Ensure warnings appear before any normal echo output
        if (m_warnOnExit)
            warnUnmeasured();
        flushEchoes();
    }

    RuntimeEvaluator::~RuntimeEvaluator() {
        m_stopGc = true;
        m_gcCv.notify_all();
        if (m_gcThread.joinable())
            m_gcThread.join();
    }

    Value RuntimeEvaluator::lookup(const std::string& name) {
        for (auto it = m_env.rbegin(); it != m_env.rend(); ++it) {
            auto fit = it->find(name);
            if (fit != it->end())
                return fit->second.value;
        }
        std::shared_ptr<Object> thisObj = currentThisObject();
        if (m_currentClassCtx) {
            if (!m_inStaticContext && thisObj) {
                RuntimeField* field = findInstanceField(m_currentClassCtx, name);
                if (field && field->offset < thisObj->fields.size())
                    return thisObj->fields[field->offset];
            }
            auto [field, owner] = findStaticFieldWithOwner(m_currentClassCtx, name);
            if (field && owner && field->offset < owner->staticStorage.size())
                return owner->staticStorage[field->offset];
        }
        auto clsIt = m_classTable.find(name);
        if (clsIt != m_classTable.end()) {
            Value v;
            v.type = Value::Type::ClassRef;
            v.classRef = clsIt->second.get();
            v.className = name;
            return v;
        }
        auto tmplIt = m_genericTemplates.find(name);
        if (tmplIt != m_genericTemplates.end()) {
            Value v;
            v.type = Value::Type::ClassRef;
            v.classRef = nullptr;
            v.className = name;
            return v;
        }
        return {};
    }

    void RuntimeEvaluator::assign(const std::string& name, const Value& v) {
        for (auto it = m_env.rbegin(); it != m_env.rend(); ++it) {
            auto fit = it->find(name);
            if (fit != it->end()) {
                Value newVal = v;
                if (fit->second.value.type == Value::Type::Object &&
                    newVal.type == Value::Type::Object && newVal.objectValue &&
                    !fit->second.value.className.empty()) {
                    newVal.className = fit->second.value.className;
                }
                fit->second.value = newVal;
                fit->second.initialized = true;
                return;
            }
        }
        std::shared_ptr<Object> thisObj = currentThisObject();
        if (m_currentClassCtx) {
            if (!m_inStaticContext && thisObj) {
                RuntimeField* field = findInstanceField(m_currentClassCtx, name);
                if (field && field->offset < thisObj->fields.size()) {
                    Value newVal = v;
                    const Value& existing = thisObj->fields[field->offset];
                    if (existing.type == Value::Type::Object &&
                        newVal.type == Value::Type::Object && newVal.objectValue &&
                        !existing.className.empty()) {
                        newVal.className = existing.className;
                    }
                    thisObj->fields[field->offset] = newVal;
                    return;
                }
            }
            auto [field, owner] = findStaticFieldWithOwner(m_currentClassCtx, name);
            if (field && owner && field->offset < owner->staticStorage.size()) {
                Value newVal = v;
                const Value& existing = owner->staticStorage[field->offset];
                if (existing.type == Value::Type::Object && newVal.type == Value::Type::Object &&
                    newVal.objectValue && !existing.className.empty()) {
                    newVal.className = existing.className;
                }
                owner->staticStorage[field->offset] = newVal;
                return;
            }
        }
        m_env.back()[name] = {v, false, true};
    }

    std::shared_ptr<Object> RuntimeEvaluator::currentThisObject() const {
        for (auto it = m_env.rbegin(); it != m_env.rend(); ++it) {
            auto found = it->find("this");
            if (found != it->end() && found->second.value.objectValue)
                return found->second.value.objectValue;
        }
        return {};
    }

    RuntimeClass* RuntimeEvaluator::findClass(const std::string& name) const {
        auto it = m_classTable.find(name);
        if (it != m_classTable.end())
            return it->second.get();
        return nullptr;
    }

    RuntimeField* RuntimeEvaluator::findInstanceField(RuntimeClass* cls, const std::string& name) {
        RuntimeClass* cur = cls;
        while (cur) {
            auto fit = cur->instanceFieldIndex.find(name);
            if (fit != cur->instanceFieldIndex.end())
                return &cur->instanceFields[fit->second];
            cur = cur->base;
        }
        return nullptr;
    }

    RuntimeField* RuntimeEvaluator::findStaticField(RuntimeClass* cls, const std::string& name) {
        RuntimeClass* cur = cls;
        while (cur) {
            auto fit = cur->staticFieldIndex.find(name);
            if (fit != cur->staticFieldIndex.end())
                return &cur->staticFields[fit->second];
            cur = cur->base;
        }
        return nullptr;
    }

    static std::string runtimeSignatureLabel(const std::string& name,
                                             const std::vector<RuntimeTypeInfo>& params) {
        std::ostringstream oss;
        oss << name << "(";
        for (size_t i = 0; i < params.size(); ++i) {
            if (i)
                oss << ",";
            if (!params[i].className.empty())
                oss << params[i].className;
            else {
                switch (params[i].kind) {
                    case Value::Type::Int:
                        oss << "int";
                        break;
                    case Value::Type::Float:
                        oss << "float";
                        break;
                    case Value::Type::Bit:
                        oss << "bit";
                        break;
                    case Value::Type::String:
                        oss << "string";
                        break;
                    case Value::Type::Char:
                        oss << "char";
                        break;
                    case Value::Type::Qubit:
                        oss << "qubit";
                        break;
                    case Value::Type::Object:
                        oss << "object";
                        break;
                    case Value::Type::ObjectArray:
                        oss << "object[]";
                        break;
                    default:
                        oss << "unknown";
                        break;
                }
            }
        }
        oss << ")";
        return oss.str();
    }

    RuntimeMethod* RuntimeEvaluator::findMethod(RuntimeClass* cls, const std::string& name,
                                                const std::vector<Value>* args) {
        auto matches = [&](const RuntimeMethod& cand, const std::vector<Value>& actuals) {
            if (cand.params.size() != actuals.size())
                return false;
            for (size_t i = 0; i < cand.params.size(); ++i) {
                const auto& exp = cand.params[i];
                const auto& act = actuals[i];
                switch (exp.kind) {
                    case Value::Type::Int:
                        if (act.type != Value::Type::Int)
                            return false;
                        break;
                    case Value::Type::Float:
                        if (act.type != Value::Type::Float)
                            return false;
                        break;
                    case Value::Type::Bit:
                        if (act.type != Value::Type::Bit)
                            return false;
                        break;
                    case Value::Type::String:
                        if (act.type != Value::Type::String)
                            return false;
                        break;
                    case Value::Type::Char:
                        if (act.type != Value::Type::Char)
                            return false;
                        break;
                    case Value::Type::Qubit:
                        if (act.type != Value::Type::Qubit)
                            return false;
                        break;
                    case Value::Type::Object:
                        if (isNullReference(act))
                            break;
                        if (act.type != Value::Type::Object)
                            return false;
                        if (!exp.className.empty() && act.className != exp.className)
                            return false;
                        break;
                    case Value::Type::ObjectArray:
                        if (act.type != Value::Type::ObjectArray)
                            return false;
                        if (!exp.className.empty() && act.className != exp.className)
                            return false;
                        break;
                    default:
                        return false;
                }
            }
            return true;
        };

        RuntimeClass* cur = cls;
        while (cur) {
            auto mit = cur->methods.find(name);
            if (mit != cur->methods.end()) {
                if (!args)
                    return mit->second.empty() ? nullptr : &mit->second.front();
                RuntimeMethod* found = nullptr;
                for (auto& cand : mit->second) {
                    if (matches(cand, *args)) {
                        if (found)
                            return nullptr;  // ambiguous
                        found = &cand;
                    }
                }
                if (found)
                    return found;
            }
            cur = cur->base;
        }
        return nullptr;
    }

    void RuntimeEvaluator::buildClassTable(Program& program) {
        m_classTable.clear();
        m_genericTemplates.clear();
        for (auto& clsNode : program.classes) {
            if (!clsNode)
                continue;
            if (!clsNode->typeParameters.empty()) {
                m_genericTemplates[clsNode->name] = clsNode.get();
                continue;
            }
            auto rc = std::make_shared<RuntimeClass>();
            rc->name = clsNode->name;
            rc->isStatic = clsNode->isStatic;
            rc->isAbstract = clsNode->isAbstract;
            m_classTable[rc->name] = rc;
        }
        // populate members
        for (auto& clsNode : program.classes) {
            if (!clsNode || !clsNode->typeParameters.empty())
                continue;  // generic templates handled lazily
            RuntimeClass* rc = findClass(clsNode->name);
            if (!rc)
                continue;
            // Wire base (non-generic class)
            if (clsNode->baseType) {
                if (auto named = dynamic_cast<NamedType*>(clsNode->baseType.get())) {
                    if (named->typeArguments.empty()) {
                        rc->base = findClass(named->nameParts.back());
                    } else {
                        rc->base = instantiateGeneric(named, {});
                    }
                }
            } else if (!clsNode->baseName.empty()) {
                rc->base = findClass(clsNode->baseName.back());
            }
            if (rc->base) {
                rc->instanceFields = rc->base->instanceFields;
                rc->instanceFieldIndex = rc->base->instanceFieldIndex;
                rc->vtable = rc->base->vtable;
            }
            for (auto& member : clsNode->members) {
                if (auto field = dynamic_cast<FieldDeclaration*>(member.get())) {
                    RuntimeField f;
                    f.name = field->name;
                    f.type = typeInfoFromAst(field->fieldType.get());
                    f.isStatic = field->isStatic;
                    f.isFinal = field->isFinal;
                    f.isTracked = field->isTracked;
                    f.initializer = field->initializer.get();
                    f.hasInitializer = field->initializer != nullptr;
                    if (auto arr = dynamic_cast<ArrayType*>(field->fieldType.get()))
                        f.arraySize = arr->size;
                    f.line = field->line;
                    f.column = field->column;
                    if (f.isTracked || f.type.kind == Value::Type::Qubit ||
                        f.type.kind == Value::Type::QubitArray)
                        rc->hasTrackedFields = true;
                    if (f.isStatic) {
                        f.offset = rc->staticFields.size();
                        rc->staticFieldIndex[f.name] = f.offset;
                        rc->staticFields.push_back(f);
                    } else {
                        f.offset = rc->instanceFields.size();
                        rc->instanceFieldIndex[f.name] = f.offset;
                        rc->instanceFields.push_back(f);
                    }
                } else if (auto method = dynamic_cast<MethodDeclaration*>(member.get())) {
                    RuntimeMethod m;
                    m.decl = method;
                    m.isStatic = method->isStatic;
                    m.isVirtual = method->isVirtual;
                    m.isOverride = method->isOverride;
                    m.owner = rc;
                    for (auto& p : method->params)
                        m.params.push_back(typeInfoFromAst(p->type.get()));
                    m.signature = runtimeSignatureLabel(method->name, m.params);
                    auto& bucket = rc->methods[method->name];
                    bucket.push_back(m);
                    RuntimeMethod* stored = &bucket.back();
                    if (stored->isVirtual || stored->isOverride) {
                        RuntimeMethod* baseMethod = nullptr;
                        if (rc->base) {
                            auto it = rc->base->methods.find(method->name);
                            if (it != rc->base->methods.end()) {
                                for (auto& cand : it->second) {
                                    if (cand.signature == stored->signature) {
                                        baseMethod = &cand;
                                        break;
                                    }
                                }
                            }
                        }
                        rc->vtable[stored->signature] = stored;
                        if (baseMethod) {
                            rc->vtable[stored->signature] = stored;
                        }
                    }
                } else if (auto ctor = dynamic_cast<ConstructorDeclaration*>(member.get())) {
                    RuntimeConstructor c;
                    c.decl = ctor;
                    for (auto& p : ctor->params) c.params.push_back(typeInfoFromAst(p->type.get()));
                    c.isDefault = ctor->isDefault;
                    rc->constructors.push_back(c);
                } else if (auto dtor = dynamic_cast<DestructorDeclaration*>(member.get())) {
                    rc->hasDestructor = true;
                    rc->destructorDecl = dtor;
                }
            }
            if (rc->staticStorage.size() < rc->staticFields.size())
                rc->staticStorage.resize(rc->staticFields.size());
        }
    }

    RuntimeClass* RuntimeEvaluator::instantiateGeneric(
        const NamedType* typeNode, const std::unordered_map<std::string, RuntimeTypeInfo>& subst) {
        if (!typeNode || typeNode->nameParts.empty())
            return nullptr;
        std::string base = typeNode->nameParts.back();
        auto tmplIt = m_genericTemplates.find(base);
        if (tmplIt == m_genericTemplates.end())
            return nullptr;
        std::vector<RuntimeTypeInfo> argInfos;
        for (auto& arg : typeNode->typeArguments)
            argInfos.push_back(typeInfoFromAst(arg.get(), subst));
        RuntimeTypeInfo assembled;
        assembled.kind = Value::Type::Object;
        assembled.className = base;
        assembled.typeArgs = argInfos;
        std::string key = typeKey(assembled);
        if (auto existing = findClass(key))
            return existing;

        core::ClassDeclaration* tmpl = tmplIt->second;
        std::unordered_map<std::string, RuntimeTypeInfo> localSubst = subst;
        for (size_t i = 0; i < tmpl->typeParameters.size() && i < argInfos.size(); ++i) {
            localSubst[tmpl->typeParameters[i]->name] = argInfos[i];
        }

        auto rc = std::make_shared<RuntimeClass>();
        rc->name = key;
        rc->isStatic = tmpl->isStatic;
        rc->isAbstract = tmpl->isAbstract;
        rc->typeArgs = argInfos;
        for (auto& tp : tmpl->typeParameters) rc->typeParamNames.push_back(tp->name);
        if (tmpl->baseType) {
            if (auto baseNamed = dynamic_cast<NamedType*>(tmpl->baseType.get())) {
                if (baseNamed->typeArguments.empty()) {
                    rc->base = findClass(baseNamed->nameParts.back());
                } else {
                    rc->base = instantiateGeneric(baseNamed, localSubst);
                }
            }
        } else if (!tmpl->baseName.empty()) {
            rc->base = findClass(tmpl->baseName.back());
        }
        if (rc->base) {
            rc->instanceFields = rc->base->instanceFields;
            rc->instanceFieldIndex = rc->base->instanceFieldIndex;
            rc->vtable = rc->base->vtable;
        }

        for (auto& member : tmpl->members) {
            if (auto field = dynamic_cast<FieldDeclaration*>(member.get())) {
                RuntimeField f;
                f.name = field->name;
                f.type = typeInfoFromAst(field->fieldType.get(), localSubst);
                f.isStatic = field->isStatic;
                f.isFinal = field->isFinal;
                f.isTracked = field->isTracked;
                f.initializer = field->initializer.get();
                f.hasInitializer = field->initializer != nullptr;
                if (auto arr = dynamic_cast<ArrayType*>(field->fieldType.get()))
                    f.arraySize = arr->size;
                f.line = field->line;
                f.column = field->column;
                if (f.isTracked || f.type.kind == Value::Type::Qubit ||
                    f.type.kind == Value::Type::QubitArray)
                    rc->hasTrackedFields = true;
                if (f.isStatic) {
                    f.offset = rc->staticFields.size();
                    rc->staticFieldIndex[f.name] = f.offset;
                    rc->staticFields.push_back(f);
                } else {
                    f.offset = rc->instanceFields.size();
                    rc->instanceFieldIndex[f.name] = f.offset;
                    rc->instanceFields.push_back(f);
                }
            } else if (auto method = dynamic_cast<MethodDeclaration*>(member.get())) {
                RuntimeMethod m;
                m.decl = method;
                m.isStatic = method->isStatic;
                m.isVirtual = method->isVirtual;
                m.isOverride = method->isOverride;
                m.owner = rc.get();
                for (auto& p : method->params)
                    m.params.push_back(typeInfoFromAst(p->type.get(), localSubst));
                m.signature = runtimeSignatureLabel(method->name, m.params);
                auto& bucket = rc->methods[method->name];
                bucket.push_back(m);
                RuntimeMethod* stored = &bucket.back();
                if (stored->isVirtual || stored->isOverride) {
                    RuntimeMethod* baseMethod = nullptr;
                    if (rc->base) {
                        auto it = rc->base->methods.find(method->name);
                        if (it != rc->base->methods.end()) {
                            for (auto& cand : it->second) {
                                if (cand.signature == stored->signature) {
                                    baseMethod = &cand;
                                    break;
                                }
                            }
                        }
                    }
                    rc->vtable[stored->signature] = stored;
                    if (baseMethod) {
                        rc->vtable[stored->signature] = stored;
                    }
                }
            } else if (auto ctor = dynamic_cast<ConstructorDeclaration*>(member.get())) {
                RuntimeConstructor c;
                c.decl = ctor;
                for (auto& p : ctor->params)
                    c.params.push_back(typeInfoFromAst(p->type.get(), localSubst));
                c.isDefault = ctor->isDefault;
                rc->constructors.push_back(c);
            } else if (auto dtor = dynamic_cast<DestructorDeclaration*>(member.get())) {
                rc->hasDestructor = true;
                rc->destructorDecl = dtor;
            }
        }
        if (rc->staticStorage.size() < rc->staticFields.size())
            rc->staticStorage.resize(rc->staticFields.size());
        m_classTable[key] = rc;
        initStaticFields(rc.get());
        return rc.get();
    }

    RuntimeClass* RuntimeEvaluator::instantiateGeneric(const NamedType* typeNode) {
        static const std::unordered_map<std::string, RuntimeTypeInfo> emptySubst;
        return instantiateGeneric(typeNode, emptySubst);
    }

    RuntimeClass* RuntimeEvaluator::instantiateGeneric(const std::string& base,
                                                       const std::vector<RuntimeTypeInfo>& args) {
        auto tmplIt = m_genericTemplates.find(base);
        if (tmplIt == m_genericTemplates.end())
            return nullptr;
        auto nt = std::make_unique<NamedType>(std::vector<std::string>{base});
        for (const auto& a : args) nt->typeArguments.push_back(runtimeTypeToAst(a));
        std::unordered_map<std::string, RuntimeTypeInfo> subst;
        core::ClassDeclaration* tmpl = tmplIt->second;
        for (size_t i = 0; i < tmpl->typeParameters.size() && i < args.size(); ++i) {
            subst[tmpl->typeParameters[i]->name] = args[i];
        }
        return instantiateGeneric(nt.get(), subst);
    }

    void RuntimeEvaluator::initStaticFields(RuntimeClass* cls) {
        if (!cls)
            return;
        for (size_t i = 0; i < cls->staticFields.size(); ++i) {
            auto& field = cls->staticFields[i];
            auto& slot = cls->staticStorage[i];
            if (slot.type != Value::Type::Void)
                continue;
            bool prevStatic = m_inStaticContext;
            auto* prevClass = m_currentClassCtx;
            m_inStaticContext = true;
            m_currentClassCtx = cls;
            slot = defaultValueForField(field, cls->name);
            if (field.hasInitializer && field.initializer) {
                slot = eval(field.initializer);
            }
            m_inStaticContext = prevStatic;
            m_currentClassCtx = prevClass;
        }
    }

    void RuntimeEvaluator::ensureGcThread() {
        if (m_gcThread.joinable())
            return;
        m_stopGc = false;
        m_gcRequested = false;
        m_gcThreadStarted = true;
        m_gcThread = std::thread([this]() {
            std::unique_lock<std::mutex> lock(m_gcMutex);
            while (!m_stopGc.load()) {
                m_gcCv.wait_for(lock, std::chrono::milliseconds(50),
                                [this]() { return m_stopGc.load(); });
                if (m_stopGc.load())
                    break;
                requestGc();
            }
        });
    }

    void RuntimeEvaluator::requestGc() { m_gcRequested = true; }

    void RuntimeEvaluator::markObject(const std::shared_ptr<Object>& obj) {
        if (!obj || obj->marked)
            return;
        obj->marked = true;
        for (const auto& field : obj->fields) markValue(field);
    }

    void RuntimeEvaluator::markValue(const Value& v) {
        if (v.type == Value::Type::Object && v.objectValue) {
            markObject(v.objectValue);
        } else if (v.type == Value::Type::ObjectArray) {
            for (const auto& o : v.objectArray) markObject(o);
        }
    }

    void RuntimeEvaluator::runCycleCollector() {
        if (!m_gcRequested.load())
            return;
        m_gcRequested = false;
        std::vector<std::shared_ptr<Object>> objects;
        {
            std::lock_guard<std::mutex> lock(m_heapMutex);
            std::vector<std::weak_ptr<Object>> alive;
            for (auto& w : m_heap) {
                if (auto obj = w.lock()) {
                    obj->marked = false;
                    objects.push_back(obj);
                    alive.push_back(obj);
                }
            }
            m_heap.swap(alive);
        }
        if (objects.empty())
            return;
        // Mark roots: environment variables and static storage
        for (const auto& scope : m_env) {
            for (const auto& kv : scope) markValue(kv.second.value);
        }
        for (const auto& kv : m_classTable) {
            const auto& cls = kv.second;
            for (const auto& v : cls->staticStorage) markValue(v);
        }
        markValue(m_returnValue);
        // Sweep unmarked non-tracked objects
        std::vector<std::shared_ptr<Object>> unreachable;
        for (auto& obj : objects) {
            if (!obj->marked && obj->cls && !obj->cls->hasTrackedFields) {
                obj->skipDestructor = true;
                unreachable.push_back(obj);
            }
        }
        for (auto& obj : unreachable) {
            for (auto& f : obj->fields) f = {};
        }
        // unreachable will drop here and be reclaimed without running destructors
        m_allocSinceGc = 0;
    }

    void RuntimeEvaluator::recordTrackedValue(const std::string& name, const Value& v) {
        if (v.type == Value::Type::Qubit) {
            int q = v.qubit;
            std::string outcome = "?";
            if (q >= 0 && q < static_cast<int>(m_lastMeasurement.size()) &&
                m_lastMeasurement[q] != -1) {
                outcome = m_lastMeasurement[q] ? "1" : "0";
            }
            m_trackedCounts[name][outcome]++;
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
            std::string outcome = "?";
            if (allMeasured) {
                for (int q : v.qubitArray) bits.push_back(m_lastMeasurement[q] ? '1' : '0');
                outcome = bits;
            }
            m_trackedCounts[name][outcome]++;
        }
    }

    void RuntimeEvaluator::destroyObject(Object* obj, bool runUserDestructor) {
        if (!obj || obj->destroyed)
            return;
        obj->destroyed = true;
        if (runUserDestructor && obj->cls) {
            bool savedReturn = m_hasReturn;
            for (RuntimeClass* cur = obj->cls; cur; cur = cur->base) {
                if (!cur->destructorDecl || !cur->destructorDecl->body)
                    continue;
                auto prevClass = m_currentClassCtx;
                auto prevStatic = m_inStaticContext;
                auto prevCtor = m_inConstructor;
                auto prevDtor = m_inDestructor;
                m_currentClassCtx = cur;
                m_inStaticContext = false;
                m_inConstructor = false;
                m_inDestructor = true;
                beginScope();
                Value thisVal;
                thisVal.type = Value::Type::Object;
                thisVal.objectValue = std::shared_ptr<Object>(obj, [](Object*) {});
                thisVal.className = cur->name;
                m_env.back()["this"] = {thisVal, false, true};
                for (auto& stmt : cur->destructorDecl->body->statements) {
                    exec(stmt.get());
                    if (m_hasReturn)
                        break;
                }
                endScope();
                m_inDestructor = prevDtor;
                m_inConstructor = prevCtor;
                m_inStaticContext = prevStatic;
                m_currentClassCtx = prevClass;
            }
            m_hasReturn = savedReturn;
        }
        // Reset tracked qubits
        if (obj->cls) {
            for (size_t i = 0; i < obj->fields.size(); ++i) {
                if (i >= obj->cls->instanceFields.size())
                    continue;
                const auto& fieldMeta = obj->cls->instanceFields[i];
                if (fieldMeta.isTracked && (obj->fields[i].type == Value::Type::Qubit ||
                                            obj->fields[i].type == Value::Type::QubitArray)) {
                    recordTrackedValue(obj->cls->name + "." + fieldMeta.name, obj->fields[i]);
                }
                if (obj->fields[i].type == Value::Type::Qubit) {
                    int q = obj->fields[i].qubit;
                    ensureQubitExists(q, fieldMeta.line, fieldMeta.column);
                    m_sim.reset(q);
                    releaseQubit(q);
                } else if (obj->fields[i].type == Value::Type::QubitArray) {
                    for (int q : obj->fields[i].qubitArray) {
                        ensureQubitExists(q, fieldMeta.line, fieldMeta.column);
                        m_sim.reset(q);
                        releaseQubit(q);
                    }
                }
            }
        }
        obj->fields.clear();
    }

    void RuntimeEvaluator::runFieldInitialisers(RuntimeClass* cls,
                                                const std::shared_ptr<Object>& obj) {
        if (!cls)
            return;
        size_t startIdx = cls->base ? cls->base->instanceFields.size() : 0;
        if (startIdx > cls->instanceFields.size())
            startIdx = cls->instanceFields.size();
        for (size_t i = startIdx; i < cls->instanceFields.size(); ++i) {
            auto& field = cls->instanceFields[i];
            if (field.isStatic)
                continue;
            auto& slot = obj->fields[field.offset];
            if (slot.type == Value::Type::Void)
                slot = defaultValueForField(field, cls->name);
            if (field.hasInitializer && field.initializer) {
                auto prevClass = m_currentClassCtx;
                bool prevStatic = m_inStaticContext;
                m_currentClassCtx = cls;
                m_inStaticContext = false;
                beginScope();
                Value thisVal;
                thisVal.type = Value::Type::Object;
                thisVal.objectValue = obj;
                thisVal.className = cls->name;
                m_env.back()["this"] = {thisVal, false, true};
                Value init = eval(field.initializer);
                slot = init;
                endScope();
                m_currentClassCtx = prevClass;
                m_inStaticContext = prevStatic;
            }
        }
    }

    namespace {
        bool argumentsMatchCtor(const std::vector<Value>& args,
                                const RuntimeConstructor& candidate) {
            if (candidate.params.size() != args.size())
                return false;
            for (size_t i = 0; i < args.size(); ++i) {
                const auto& expected = candidate.params[i];
                const auto& actual = args[i];
                if (expected.kind == Value::Type::Object) {
                    if (isNullReference(actual))
                        continue;
                    if (actual.type != Value::Type::Object)
                        return false;
                    if (!expected.className.empty() && actual.className != expected.className)
                        return false;
                } else if (expected.kind == Value::Type::ObjectArray) {
                    if (actual.type != Value::Type::ObjectArray)
                        return false;
                    if (!expected.className.empty() && actual.className != expected.className)
                        return false;
                } else if (expected.kind != actual.type) {
                    return false;
                }
            }
            return true;
        }
    }  // namespace

    void RuntimeEvaluator::runConstructorChain(RuntimeClass* cls,
                                               const std::shared_ptr<Object>& obj,
                                               ConstructorDeclaration* ctor,
                                               const std::vector<Value>& args) {
        if (!cls)
            return;

        if (kTraceConstructors) {
            std::cerr << "[ctor] " << cls->name << " args=" << args.size() << std::endl;
        }

        bool savedReturn = m_hasReturn;
        m_hasReturn = false;

        // Prepare context for this constructor (needed to evaluate explicit super(...) args).
        auto prevClass = m_currentClassCtx;
        bool prevStatic = m_inStaticContext;
        bool prevCtor = m_inConstructor;
        bool prevDtor = m_inDestructor;
        m_currentClassCtx = cls;
        m_inStaticContext = false;
        m_inConstructor = true;
        m_inDestructor = false;
        beginScope();
        Value thisVal;
        thisVal.type = Value::Type::Object;
        thisVal.objectValue = obj;
        thisVal.className = cls->name;
        m_env.back()["this"] = {thisVal, false, true};
        for (size_t i = 0; ctor && i < ctor->params.size() && i < args.size(); ++i) {
            m_env.back()[ctor->params[i]->name] = {args[i], false, true};
        }

        // Detect an explicit super(...) call as the first statement.
        std::vector<Value> superArgs;
        ConstructorDeclaration* superCtorDecl = nullptr;
        bool hasExplicitSuper = false;
        if (ctor && ctor->body && !ctor->body->statements.empty()) {
            if (auto exprStmt =
                    dynamic_cast<ExpressionStatement*>(ctor->body->statements[0].get())) {
                if (auto call = dynamic_cast<CallExpression*>(exprStmt->expression.get())) {
                    if (dynamic_cast<SuperExpression*>(call->callee.get())) {
                        hasExplicitSuper = true;
                        for (auto& a : call->arguments) superArgs.push_back(eval(a.get()));
                    }
                }
            }
        }

        // Dispatch to base constructor (explicit or default).
        if (cls->base) {
            if (hasExplicitSuper) {
                for (auto& c : cls->base->constructors) {
                    if (argumentsMatchCtor(superArgs, c)) {
                        superCtorDecl = c.decl;
                        break;
                    }
                }
                if (!superCtorDecl) {
                    throw BlochError(ErrorCategory::Runtime, ctor ? ctor->line : 0,
                                     ctor ? ctor->column : 0,
                                     "no matching base constructor for 'super(...)'");
                }
            } else {
                // Implicit default base constructor
                for (auto& c : cls->base->constructors) {
                    if (c.params.empty()) {
                        superCtorDecl = c.decl;
                        break;
                    }
                }
            }
            if (kTraceConstructors) {
                std::cerr << "[ctor] " << cls->name << " -> base " << cls->base->name
                          << " args=" << superArgs.size() << std::endl;
            }
            runConstructorChain(cls->base, obj, superCtorDecl, superArgs);
        }

        // Initialise fields before running the body.
        if (kTraceConstructors) {
            std::cerr << "[ctor] " << cls->name << " init fields" << std::endl;
        }
        runFieldInitialisers(cls, obj);

        if (ctor && ctor->isDefault) {
            // Default constructor: bind parameters to matching fields.
            for (size_t i = 0; i < ctor->params.size() && i < args.size(); ++i) {
                const auto& param = ctor->params[i];
                auto fieldMeta = findInstanceField(cls, param->name);
                if (fieldMeta && fieldMeta->offset < obj->fields.size()) {
                    obj->fields[fieldMeta->offset] = args[i];
                }
            }
        }

        if (ctor && ctor->body) {
            size_t startIdx = hasExplicitSuper ? 1 : 0;  // already handled
            if (kTraceConstructors) {
                std::cerr << "[ctor] " << cls->name << " execute body (skip=" << startIdx << ")"
                          << std::endl;
            }
            for (size_t i = startIdx; i < ctor->body->statements.size(); ++i) {
                exec(ctor->body->statements[i].get());
                if (m_hasReturn)
                    break;
            }
        }
        if (kTraceConstructors) {
            std::cerr << "[ctor] " << cls->name << " done" << std::endl;
        }

        endScope();
        m_currentClassCtx = prevClass;
        m_inStaticContext = prevStatic;
        m_inConstructor = prevCtor;
        m_inDestructor = prevDtor;
        m_hasReturn = savedReturn;
    }

    Value RuntimeEvaluator::callMethod(RuntimeMethod* method, RuntimeClass* staticDispatchClass,
                                       const std::shared_ptr<Object>& receiver,
                                       const std::vector<Value>& args) {
        if (!method || !method->decl)
            return {};
        auto prevClass = m_currentClassCtx;
        bool prevStatic = m_inStaticContext;
        bool prevCtor = m_inConstructor;
        bool prevDtor = m_inDestructor;
        m_currentClassCtx = staticDispatchClass ? staticDispatchClass : method->owner;
        m_inStaticContext = method->isStatic;
        m_inConstructor = false;
        m_inDestructor = false;
        beginScope();
        if (!method->isStatic) {
            Value thisVal;
            thisVal.type = Value::Type::Object;
            thisVal.objectValue = receiver;
            thisVal.className = method->owner ? method->owner->name : "";
            m_env.back()["this"] = {thisVal, false, true};
        }
        m_returnValue = {};
        for (size_t i = 0; i < method->decl->params.size() && i < args.size(); ++i) {
            m_env.back()[method->decl->params[i]->name] = {args[i], false, true};
        }
        bool prevReturn = m_hasReturn;
        m_hasReturn = false;
        if (method->decl->body) {
            for (auto& stmt : method->decl->body->statements) {
                exec(stmt.get());
                if (m_hasReturn)
                    break;
            }
        }
        Value ret = m_returnValue;
        endScope();
        m_hasReturn = prevReturn;
        m_currentClassCtx = prevClass;
        m_inStaticContext = prevStatic;
        m_inConstructor = prevCtor;
        m_inDestructor = prevDtor;
        return ret;
    }

    Value RuntimeEvaluator::call(FunctionDeclaration* fn, const std::vector<Value>& args) {
        // Bind parameters, run the body until a return is hit, then unwind.
        beginScope();
        for (size_t i = 0; i < fn->params.size() && i < args.size(); ++i) {
            m_env.back()[fn->params[i]->name] = {args[i], false, true};
        }
        bool prevReturn = m_hasReturn;
        m_returnValue = {};
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
        if (m_gcRequested.load())
            runCycleCollector();
        if (!s)
            return;
        auto isTruthy = [](const Value& v) {
            switch (v.type) {
                case Value::Type::Boolean:
                    return v.boolValue;
                case Value::Type::Bit:
                    return v.bitValue != 0;
                case Value::Type::Int:
                    return v.intValue != 0;
                case Value::Type::Float:
                    return v.floatValue != 0.0;
                default:
                    return false;
            }
        };
        if (auto var = dynamic_cast<VariableDeclaration*>(s)) {
            Value v;
            if (auto prim = dynamic_cast<PrimitiveType*>(var->varType.get())) {
                if (prim->name == "int")
                    v.type = Value::Type::Int;
                else if (prim->name == "bit")
                    v.type = Value::Type::Bit;
                else if (prim->name == "boolean")
                    v.type = Value::Type::Boolean;
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
                    else if (elem->name == "boolean")
                        v.type = Value::Type::BooleanArray;
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
                if (arr->size < 0 && arr->sizeExpression) {
                    Value sizeVal = eval(arr->sizeExpression.get());
                    if (sizeVal.type != Value::Type::Int) {
                        throw BlochError(ErrorCategory::Runtime, var->line, var->column,
                                         "array size must evaluate to an int");
                    }
                    arr->size = sizeVal.intValue;
                    if (arr->size < 0) {
                        throw BlochError(ErrorCategory::Runtime, var->line, var->column,
                                         "array size must be non-negative");
                    }
                }
                // Handle fixed-size allocation (without initializer)
                if (arr->size >= 0 && !var->initializer) {
                    int n = arr->size;
                    if (v.type == Value::Type::BitArray)
                        v.bitArray.assign(n, 0);
                    else if (v.type == Value::Type::BooleanArray)
                        v.boolArray.assign(n, false);
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
            } else if (dynamic_cast<NamedType*>(var->varType.get())) {
                v.type = Value::Type::Object;
            }
            bool initialized = false;
            if (var->initializer) {
                // Special case of array literal initialisation for typed arrays
                if (auto arrType = dynamic_cast<ArrayType*>(var->varType.get())) {
                    if (auto elem = dynamic_cast<PrimitiveType*>(arrType->elementType.get())) {
                        if (elem->name == "qubit") {
                            throw BlochError(ErrorCategory::Runtime, var->line, var->column,
                                             "qubit[] cannot be initialised");
                        }
                        if (auto arrLit =
                                dynamic_cast<ArrayLiteralExpression*>(var->initializer.get())) {
                            // If size is specified, enforce it
                            if (arrType->size >= 0 &&
                                static_cast<int>(arrLit->elements.size()) != arrType->size) {
                                throw BlochError(
                                    ErrorCategory::Runtime, var->line, var->column,
                                    "array initializer length does not match declared size");
                            }
                            if (elem->name == "bit") {
                                v.type = Value::Type::BitArray;
                                v.bitArray.clear();
                                for (auto& el : arrLit->elements) {
                                    Value ev = eval(el.get());
                                    if (ev.type != Value::Type::Bit)
                                        throw BlochError(ErrorCategory::Runtime, el->line,
                                                         el->column,
                                                         "bit[] initialiser expects bit elements");
                                    v.bitArray.push_back(ev.bitValue ? 1 : 0);
                                }
                            } else if (elem->name == "boolean") {
                                v.type = Value::Type::BooleanArray;
                                v.boolArray.clear();
                                for (auto& el : arrLit->elements) {
                                    Value ev = eval(el.get());
                                    if (ev.type == Value::Type::Boolean)
                                        v.boolArray.push_back(ev.boolValue);
                                    else if (ev.type == Value::Type::Bit)
                                        v.boolArray.push_back(ev.bitValue != 0);
                                    else
                                        throw BlochError(
                                            ErrorCategory::Runtime, el->line, el->column,
                                            "boolean[] initialiser expects boolean elements");
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
                                            ErrorCategory::Runtime, el->line, el->column,
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
                                            ErrorCategory::Runtime, el->line, el->column,
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
                                            ErrorCategory::Runtime, el->line, el->column,
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
                                            ErrorCategory::Runtime, el->line, el->column,
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
            if (isTruthy(cond)) {
                exec(ifs->thenBranch.get());
            } else {
                exec(ifs->elseBranch.get());
            }
        } else if (auto tern = dynamic_cast<TernaryStatement*>(s)) {
            Value cond = eval(tern->condition.get());
            if (isTruthy(cond)) {
                exec(tern->thenBranch.get());
            } else {
                exec(tern->elseBranch.get());
            }
        } else if (auto fors = dynamic_cast<ForStatement*>(s)) {
            beginScope();
            if (fors->initializer)
                exec(fors->initializer.get());
            while (true) {
                bool condVal = true;
                if (fors->condition)
                    condVal = isTruthy(eval(fors->condition.get()));
                if (!condVal)
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
                bool condVal = true;
                if (whiles->condition)
                    condVal = isTruthy(eval(whiles->condition.get()));
                if (!condVal)
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
            ensureQubitExists(q.qubit, reset->line, reset->column);
            m_sim.reset(q.qubit);
            unmarkMeasured(q.qubit);
        } else if (auto meas = dynamic_cast<MeasureStatement*>(s)) {
            Value q = eval(meas->qubit.get());
            if (q.type == Value::Type::QubitArray) {
                for (int idx = 0; idx < static_cast<int>(q.qubitArray.size()); ++idx) {
                    int qid = q.qubitArray[idx];
                    ensureQubitActive(qid, meas->line, meas->column);
                    int bit = m_sim.measure(qid);
                    markMeasured(qid);
                    if (qid >= 0 && qid < static_cast<int>(m_lastMeasurement.size()))
                        m_lastMeasurement[qid] = bit;
                }
            } else {
                ensureQubitActive(q.qubit, meas->line, meas->column);
                int bit = m_sim.measure(q.qubit);
                markMeasured(q.qubit);
                if (q.qubit >= 0 && q.qubit < static_cast<int>(m_lastMeasurement.size()))
                    m_lastMeasurement[q.qubit] = bit;
            }
        } else if (auto destroy = dynamic_cast<DestroyStatement*>(s)) {
            if (auto var = dynamic_cast<VariableExpression*>(destroy->target.get())) {
                assign(var->name, {});
                requestGc();
            } else if (auto mem = dynamic_cast<MemberAccessExpression*>(destroy->target.get())) {
                Value obj = eval(mem->object.get());
                if (obj.type == Value::Type::Object && obj.objectValue) {
                    RuntimeField* field = obj.objectValue->cls
                                              ? findInstanceField(obj.objectValue->cls, mem->member)
                                              : nullptr;
                    if (field) {
                        if (field->offset < obj.objectValue->fields.size())
                            obj.objectValue->fields[field->offset] = {};
                        requestGc();
                    }
                }
            } else {
                (void)eval(destroy->target.get());
            }
        } else if (auto assignStmt = dynamic_cast<AssignmentStatement*>(s)) {
            Value val = eval(assignStmt->value.get());
            assign(assignStmt->name, val);
        }
    }

    Value RuntimeEvaluator::eval(Expression* e) {
        if (!e)
            return {};
        if (dynamic_cast<NullLiteralExpression*>(e)) {
            Value v;
            v.type = Value::Type::Object;
            v.objectValue = nullptr;
            v.className = "";
            return v;
        }
        if (auto lit = dynamic_cast<LiteralExpression*>(e)) {
            Value v;
            if (lit->literalType == "bit") {
                v.type = Value::Type::Bit;
                v.bitValue = std::stoi(lit->value);
            } else if (lit->literalType == "boolean") {
                v.type = Value::Type::Boolean;
                v.boolValue = (lit->value == "true");
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
        } else if (auto paren = dynamic_cast<ParenthesizedExpression*>(e)) {
            return eval(paren->expression.get());
        } else if (auto cast = dynamic_cast<CastExpression*>(e)) {
            Value in = eval(cast->expression.get());
            RuntimeTypeInfo target = typeInfoFromAst(cast->targetType.get());
            switch (target.kind) {
                case Value::Type::Int: {
                    Value v;
                    v.type = Value::Type::Int;
                    if (in.type == Value::Type::Int) {
                        v.intValue = in.intValue;
                        return v;
                    }
                    if (in.type == Value::Type::Bit) {
                        v.intValue = in.bitValue;
                        return v;
                    }
                    if (in.type == Value::Type::Float) {
                        v.intValue = static_cast<int>(in.floatValue);
                        return v;
                    }
                    if (in.type == Value::Type::Char) {
                        v.intValue = static_cast<int>(in.charValue);
                        return v;
                    }
                    break;
                }
                case Value::Type::Float: {
                    Value v;
                    v.type = Value::Type::Float;
                    if (in.type == Value::Type::Float) {
                        v.floatValue = in.floatValue;
                        return v;
                    }
                    if (in.type == Value::Type::Int) {
                        v.floatValue = static_cast<double>(in.intValue);
                        return v;
                    }
                    if (in.type == Value::Type::Bit) {
                        v.floatValue = static_cast<double>(in.bitValue);
                        return v;
                    }
                    if (in.type == Value::Type::Char) {
                        v.floatValue = static_cast<double>(in.charValue);
                        return v;
                    }
                    break;
                }
                case Value::Type::Bit: {
                    Value v;
                    v.type = Value::Type::Bit;
                    if (in.type == Value::Type::Bit) {
                        v.bitValue = in.bitValue;
                        return v;
                    }
                    if (in.type == Value::Type::Int) {
                        v.bitValue = in.intValue != 0 ? 1 : 0;
                        return v;
                    }
                    if (in.type == Value::Type::Float) {
                        v.bitValue = in.floatValue != 0.0 ? 1 : 0;
                        return v;
                    }
                    break;
                }
                case Value::Type::Char: {
                    Value v;
                    v.type = Value::Type::Char;
                    if (in.type == Value::Type::Char) {
                        v.charValue = in.charValue;
                        return v;
                    }
                    if (in.type == Value::Type::Int) {
                        v.charValue = static_cast<char>(in.intValue);
                        return v;
                    }
                    break;
                }
                default:
                    break;
            }
            throw BlochError(ErrorCategory::Runtime, cast->line, cast->column,
                             "invalid cast operation");
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
                            throw BlochError(ErrorCategory::Runtime, el->line, el->column,
                                             "inconsistent element types in array literal");
                        v.bitArray.push_back(ev.bitValue ? 1 : 0);
                    }
                    break;
                case Value::Type::Boolean:
                    v.type = Value::Type::BooleanArray;
                    for (auto& el : arr->elements) {
                        Value ev = eval(el.get());
                        if (ev.type != Value::Type::Boolean)
                            throw BlochError(ErrorCategory::Runtime, el->line, el->column,
                                             "inconsistent element types in array literal");
                        v.boolArray.push_back(ev.boolValue);
                    }
                    break;
                case Value::Type::Int:
                    v.type = Value::Type::IntArray;
                    for (auto& el : arr->elements) {
                        Value ev = eval(el.get());
                        if (ev.type != Value::Type::Int && ev.type != Value::Type::Bit)
                            throw BlochError(ErrorCategory::Runtime, el->line, el->column,
                                             "inconsistent element types in array literal");
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
                            throw BlochError(ErrorCategory::Runtime, el->line, el->column,
                                             "inconsistent element types in array literal");
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
                            throw BlochError(ErrorCategory::Runtime, el->line, el->column,
                                             "inconsistent element types in array literal");
                        v.stringArray.push_back(ev.stringValue);
                    }
                    break;
                case Value::Type::Char:
                    v.type = Value::Type::CharArray;
                    for (auto& el : arr->elements) {
                        Value ev = eval(el.get());
                        if (ev.type != Value::Type::Char)
                            throw BlochError(ErrorCategory::Runtime, el->line, el->column,
                                             "inconsistent element types in array literal");
                        v.charArray.push_back(ev.charValue);
                    }
                    break;
                default:
                    throw BlochError(ErrorCategory::Runtime, arr->line, arr->column,
                                     "unsupported array literal type");
            }
            return v;
        } else if (auto thisExpr = dynamic_cast<ThisExpression*>(e)) {
            return lookup("this");
        } else if (auto superExpr = dynamic_cast<SuperExpression*>(e)) {
            (void)superExpr;
            Value v;
            v.type = Value::Type::ClassRef;
            if (m_currentClassCtx && m_currentClassCtx->base) {
                v.classRef = m_currentClassCtx->base;
                v.className = m_currentClassCtx->base->name;
            }
            return v;
        } else if (auto newExpr = dynamic_cast<NewExpression*>(e)) {
            std::unordered_map<std::string, RuntimeTypeInfo> subst;
            if (m_currentClassCtx && !m_currentClassCtx->typeParamNames.empty()) {
                for (size_t i = 0; i < m_currentClassCtx->typeParamNames.size() &&
                                   i < m_currentClassCtx->typeArgs.size();
                     ++i) {
                    subst[m_currentClassCtx->typeParamNames[i]] = m_currentClassCtx->typeArgs[i];
                }
            }
            RuntimeTypeInfo tinfo = typeInfoFromAst(newExpr->classType.get(), subst);
            RuntimeClass* cls = findClass(tinfo.className);
            if (!cls) {
                if (auto named = dynamic_cast<NamedType*>(newExpr->classType.get()))
                    cls = instantiateGeneric(named, subst);
            }
            if (!cls)
                throw BlochError(ErrorCategory::Runtime, newExpr->line, newExpr->column,
                                 "class '" + tinfo.className + "' not found");
            if (cls->isStatic || cls->isAbstract) {
                throw BlochError(ErrorCategory::Runtime, newExpr->line, newExpr->column,
                                 "cannot instantiate static or abstract class '" + cls->name + "'");
            }
            auto deleter = [this](Object* obj) {
                destroyObject(obj, !obj->skipDestructor);
                delete obj;
            };
            auto obj = std::shared_ptr<Object>(new Object{}, deleter);
            obj->cls = cls;
            obj->owner = this;
            obj->fields.assign(cls->instanceFields.size(), {});
            for (auto& f : cls->instanceFields) {
                if (!f.isStatic && f.offset < obj->fields.size())
                    obj->fields[f.offset] = defaultValueForField(f, cls->name);
            }
            {
                std::lock_guard<std::mutex> lock(m_heapMutex);
                m_heap.push_back(obj);
            }
            std::vector<Value> args;
            for (auto& a : newExpr->arguments) args.push_back(eval(a.get()));
            ConstructorDeclaration* ctorDecl = nullptr;
            for (auto& c : cls->constructors) {
                if (argumentsMatchCtor(args, c)) {
                    ctorDecl = c.decl;
                    break;
                }
            }
            if (!ctorDecl) {
                throw BlochError(ErrorCategory::Runtime, newExpr->line, newExpr->column,
                                 "no constructor matches provided arguments");
            }
            if (kTraceConstructors) {
                std::cerr << "[new] " << cls->name << " selected ctor params=" << args.size()
                          << std::endl;
            }
            runConstructorChain(cls, obj, ctorDecl, args);
            Value v;
            v.type = Value::Type::Object;
            v.objectValue = obj;
            v.className = cls->name;
            ++m_allocSinceGc;
            if (m_allocSinceGc > 16)
                requestGc();
            return v;
        } else if (auto memAcc = dynamic_cast<MemberAccessExpression*>(e)) {
            Value obj = eval(memAcc->object.get());
            if (isNullReference(obj)) {
                throw BlochError(ErrorCategory::Runtime, memAcc->line, memAcc->column,
                                 "null reference");
            }
            if (obj.type == Value::Type::ClassRef && obj.classRef) {
                auto [field, owner] = findStaticFieldWithOwner(obj.classRef, memAcc->member);
                RuntimeMethod* method = findMethod(obj.classRef, memAcc->member);
                if (field && owner) {
                    size_t idx = field->offset;
                    if (idx < owner->staticStorage.size())
                        return owner->staticStorage[idx];
                } else if (method) {
                    Value v;
                    v.type = Value::Type::ClassRef;
                    v.classRef = obj.classRef;
                    v.className = obj.classRef->name;
                    return v;
                }
                throw BlochError(ErrorCategory::Runtime, memAcc->line, memAcc->column,
                                 "member not found on class");
            }
            if (obj.type == Value::Type::Object && obj.objectValue) {
                RuntimeField* instField =
                    obj.objectValue->cls ? findInstanceField(obj.objectValue->cls, memAcc->member)
                                         : nullptr;
                if (instField) {
                    if (instField->offset < obj.objectValue->fields.size())
                        return obj.objectValue->fields[instField->offset];
                } else {
                    auto [staticField, owner] =
                        obj.objectValue->cls
                            ? findStaticFieldWithOwner(obj.objectValue->cls, memAcc->member)
                            : std::pair<RuntimeField*, RuntimeClass*>{nullptr, nullptr};
                    if (staticField && owner && staticField->offset < owner->staticStorage.size())
                        return owner->staticStorage[staticField->offset];
                }
            }
            return {};
        } else if (auto bin = dynamic_cast<BinaryExpression*>(e)) {
            Value l = eval(bin->left.get());
            Value r = eval(bin->right.get());
            auto isObjectLike = [](const Value& v) {
                return v.type == Value::Type::Object || v.type == Value::Type::ClassRef;
            };
            auto makeBoolean = [](bool b) {
                Value v;
                v.type = Value::Type::Boolean;
                v.boolValue = b;
                return v;
            };

            if (bin->op == "==" || bin->op == "!=") {
                if (isObjectLike(l) || isObjectLike(r)) {
                    if (l.type == Value::Type::Object && r.type == Value::Type::Object) {
                        bool eq = l.objectValue == r.objectValue;
                        bool res = (bin->op == "==") ? eq : !eq;
                        return makeBoolean(res);
                    }
                    if (l.type == Value::Type::ClassRef && r.type == Value::Type::ClassRef) {
                        bool eq = l.classRef == r.classRef;
                        bool res = (bin->op == "==") ? eq : !eq;
                        return makeBoolean(res);
                    }
                    throw BlochError(ErrorCategory::Runtime, bin->line, bin->column,
                                     "equality on references requires two class references");
                }
            }

            bool lIsBool = l.type == Value::Type::Boolean;
            bool rIsBool = r.type == Value::Type::Boolean;
            if (lIsBool || rIsBool) {
                auto toBool = [&](const Value& v) -> bool {
                    if (v.type == Value::Type::Boolean)
                        return v.boolValue;
                    if (v.type == Value::Type::Bit)
                        return v.bitValue != 0;
                    throw BlochError(ErrorCategory::Runtime, bin->line, bin->column,
                                     "boolean operations require boolean or bit operands");
                };
                if (bin->op == "&&" || bin->op == "||") {
                    bool lb = toBool(l);
                    bool rb = toBool(r);
                    return makeBoolean(bin->op == "&&" ? (lb && rb) : (lb || rb));
                }
                if (bin->op == "==" || bin->op == "!=") {
                    bool lb = toBool(l);
                    bool rb = toBool(r);
                    return makeBoolean(bin->op == "==" ? (lb == rb) : (lb != rb));
                }
                throw BlochError(ErrorCategory::Runtime, bin->line, bin->column,
                                 "operator '" + bin->op + "' not supported for boolean");
            }

            auto lInt = l.type == Value::Type::Bit ? l.bitValue : l.intValue;
            auto rInt = r.type == Value::Type::Bit ? r.bitValue : r.intValue;
            double lNum = l.type == Value::Type::Float ? l.floatValue : static_cast<double>(lInt);
            double rNum = r.type == Value::Type::Float ? r.floatValue : static_cast<double>(rInt);
            if (bin->op == "+") {
                if (l.type == Value::Type::String || r.type == Value::Type::String) {
                    return {Value::Type::String, 0, 0.0, 0, valueToString(l) + valueToString(r)};
                }
                if (isObjectLike(l) || isObjectLike(r)) {
                    throw BlochError(ErrorCategory::Runtime, bin->line, bin->column,
                                     "operator '+' not supported for class references");
                }
                if (l.type == Value::Type::Float || r.type == Value::Type::Float) {
                    return {Value::Type::Float, 0, lNum + rNum};
                }
                return {Value::Type::Int, lInt + rInt};
            }
            if (isObjectLike(l) || isObjectLike(r)) {
                throw BlochError(ErrorCategory::Runtime, bin->line, bin->column,
                                 "operator '" + bin->op + "' not supported for class references");
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
                    throw BlochError(ErrorCategory::Runtime, bin->line, bin->column,
                                     "division by zero");
                }
                return {Value::Type::Float, 0, lNum / rNum};
            }
            if (bin->op == "%") {
                if (rInt == 0) {
                    throw BlochError(ErrorCategory::Runtime, bin->line, bin->column,
                                     "modulo by zero");
                }
                return {Value::Type::Int, lInt % rInt};
            }

            if (bin->op == ">") {
                if (l.type == Value::Type::Float || r.type == Value::Type::Float)
                    return makeBoolean(lNum > rNum);
                return makeBoolean(lInt > rInt);
            }
            if (bin->op == "<") {
                if (l.type == Value::Type::Float || r.type == Value::Type::Float)
                    return makeBoolean(lNum < rNum);
                return makeBoolean(lInt < rInt);
            }
            if (bin->op == ">=") {
                if (l.type == Value::Type::Float || r.type == Value::Type::Float)
                    return makeBoolean(lNum >= rNum);
                return makeBoolean(lInt >= rInt);
            }
            if (bin->op == "<=") {
                if (l.type == Value::Type::Float || r.type == Value::Type::Float)
                    return makeBoolean(lNum <= rNum);
                return makeBoolean(lInt <= rInt);
            }
            if (bin->op == "==") {
                if (l.type == Value::Type::Float || r.type == Value::Type::Float)
                    return makeBoolean(lNum == rNum);
                return makeBoolean(lInt == rInt);
            }
            if (bin->op == "!=") {
                if (l.type == Value::Type::Float || r.type == Value::Type::Float)
                    return makeBoolean(lNum != rNum);
                return makeBoolean(lInt != rInt);
            }
            if (bin->op == "&&") {
                bool lb = l.type == Value::Type::Float ? lNum != 0.0 : lInt != 0;
                bool rb = r.type == Value::Type::Float ? rNum != 0.0 : rInt != 0;
                return makeBoolean(lb && rb);
            }
            if (bin->op == "||") {
                bool lb = l.type == Value::Type::Float ? lNum != 0.0 : lInt != 0;
                bool rb = r.type == Value::Type::Float ? rNum != 0.0 : rInt != 0;
                return makeBoolean(lb || rb);
            }
            if (bin->op == "&") {
                if (l.type == Value::Type::Bit && r.type == Value::Type::Bit)
                    return {Value::Type::Bit, 0, 0.0, l.bitValue & r.bitValue};
                if (l.type == Value::Type::BitArray && r.type == Value::Type::BitArray) {
                    if (l.bitArray.size() != r.bitArray.size()) {
                        throw BlochError(ErrorCategory::Runtime, bin->line, bin->column,
                                         "bit arrays must be same length for '&'");
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
                throw BlochError(ErrorCategory::Runtime, bin->line, bin->column,
                                 "bitwise '&' requires bit or bit[] operands");
            }
            if (bin->op == "|") {
                if (l.type == Value::Type::Bit && r.type == Value::Type::Bit)
                    return {Value::Type::Bit, 0, 0.0, l.bitValue | r.bitValue};
                if (l.type == Value::Type::BitArray && r.type == Value::Type::BitArray) {
                    if (l.bitArray.size() != r.bitArray.size()) {
                        throw BlochError(ErrorCategory::Runtime, bin->line, bin->column,
                                         "bit arrays must be same length for '|'");
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
                throw BlochError(ErrorCategory::Runtime, bin->line, bin->column,
                                 "bitwise '|' requires bit or bit[] operands");
            }
            if (bin->op == "^") {
                if (l.type == Value::Type::Bit && r.type == Value::Type::Bit)
                    return {Value::Type::Bit, 0, 0.0, l.bitValue ^ r.bitValue};
                if (l.type == Value::Type::BitArray && r.type == Value::Type::BitArray) {
                    if (l.bitArray.size() != r.bitArray.size()) {
                        throw BlochError(ErrorCategory::Runtime, bin->line, bin->column,
                                         "bit arrays must be same length for '^'");
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
                throw BlochError(ErrorCategory::Runtime, bin->line, bin->column,
                                 "bitwise '^' requires bit or bit[] operands");
            }
        } else if (auto unary = dynamic_cast<UnaryExpression*>(e)) {
            Value r = eval(unary->right.get());
            if (unary->op == "-") {
                if (r.type == Value::Type::Float)
                    return {Value::Type::Float, 0, -r.floatValue};
                return {Value::Type::Int, -r.intValue};
            }
            if (unary->op == "!") {
                if (r.type == Value::Type::BitArray || r.type == Value::Type::BooleanArray) {
                    throw BlochError(ErrorCategory::Runtime, unary->line, unary->column,
                                     "logical '!' unsupported for bit[] or boolean[]");
                }
                bool rb = false;
                if (r.type == Value::Type::Boolean)
                    rb = r.boolValue;
                else if (r.type == Value::Type::Float)
                    rb = r.floatValue != 0.0;
                else if (r.type == Value::Type::Bit)
                    rb = r.bitValue != 0;
                else if (r.type == Value::Type::Int)
                    rb = r.intValue != 0;
                Value v;
                v.type = Value::Type::Boolean;
                v.boolValue = !rb;
                return v;
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
                throw BlochError(ErrorCategory::Runtime, unary->line, unary->column,
                                 "bitwise '~' requires bit or bit[] operand");
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
                    if (name == "h") {
                        ensureQubitActive(args[0].qubit, callExpr->line, callExpr->column);
                        m_sim.h(args[0].qubit);
                    } else if (name == "x") {
                        ensureQubitActive(args[0].qubit, callExpr->line, callExpr->column);
                        m_sim.x(args[0].qubit);
                    } else if (name == "y") {
                        ensureQubitActive(args[0].qubit, callExpr->line, callExpr->column);
                        m_sim.y(args[0].qubit);
                    } else if (name == "z") {
                        ensureQubitActive(args[0].qubit, callExpr->line, callExpr->column);
                        m_sim.z(args[0].qubit);
                    } else if (name == "rx") {
                        ensureQubitActive(args[0].qubit, callExpr->line, callExpr->column);
                        m_sim.rx(args[0].qubit, args[1].floatValue);
                    } else if (name == "ry") {
                        ensureQubitActive(args[0].qubit, callExpr->line, callExpr->column);
                        m_sim.ry(args[0].qubit, args[1].floatValue);
                    } else if (name == "rz") {
                        ensureQubitActive(args[0].qubit, callExpr->line, callExpr->column);
                        m_sim.rz(args[0].qubit, args[1].floatValue);
                    } else if (name == "cx") {
                        ensureQubitActive(args[0].qubit, callExpr->line, callExpr->column);
                        ensureQubitActive(args[1].qubit, callExpr->line, callExpr->column);
                        m_sim.cx(args[0].qubit, args[1].qubit);
                    }
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
                RuntimeMethod* method = nullptr;
                RuntimeClass* staticCls = m_currentClassCtx;
                if (staticCls)
                    method = findMethod(staticCls, name, &args);
                if (method) {
                    std::shared_ptr<Object> receiver;
                    if (!method->isStatic) {
                        receiver = currentThisObject();
                        if (!receiver) {
                            throw BlochError(
                                ErrorCategory::Runtime, callExpr->line, callExpr->column,
                                "instance method '" + name + "' requires an object receiver");
                        }
                    }
                    return callMethod(method, staticCls, receiver, args);
                }
            } else if (auto member =
                           dynamic_cast<MemberAccessExpression*>(callExpr->callee.get())) {
                std::vector<Value> args;
                for (auto& a : callExpr->arguments) args.push_back(eval(a.get()));
                Value target = eval(member->object.get());
                if (isNullReference(target)) {
                    throw BlochError(ErrorCategory::Runtime, member->line, member->column,
                                     "null reference");
                }
                RuntimeMethod* method = nullptr;
                RuntimeClass* staticCls = nullptr;
                std::shared_ptr<Object> receiver;
                bool viaSuper = dynamic_cast<SuperExpression*>(member->object.get()) != nullptr;
                if (target.type == Value::Type::Object && target.objectValue) {
                    receiver = target.objectValue;
                    staticCls = !target.className.empty() ? findClass(target.className) : nullptr;
                    if (!staticCls)
                        staticCls = receiver->cls;
                    method = findMethod(staticCls, member->member, &args);
                    if (method && method->isVirtual && receiver->cls) {
                        auto it = receiver->cls->vtable.find(method->signature);
                        if (it != receiver->cls->vtable.end())
                            method = it->second;
                    }
                    if (viaSuper && staticCls && staticCls->base) {
                        method = findMethod(staticCls->base, member->member, &args);
                        staticCls = staticCls->base;
                    }
                } else if (target.type == Value::Type::ClassRef && target.classRef) {
                    staticCls = target.classRef;
                    method = findMethod(staticCls, member->member, &args);
                } else if (target.type == Value::Type::ClassRef && !target.classRef &&
                           !target.className.empty()) {
                    // Static call on a generic template (e.g., List.of(x)) — attempt to
                    // instantiate using argument types.
                    std::vector<RuntimeTypeInfo> inferred;
                    if (!args.empty())
                        inferred.push_back(typeInfoFromValue(args.front()));
                    staticCls = instantiateGeneric(target.className, inferred);
                    if (staticCls)
                        method = findMethod(staticCls, member->member, &args);
                }
                if (method) {
                    return callMethod(method, staticCls, receiver, args);
                }
            }
        } else if (auto idx = dynamic_cast<MeasureExpression*>(e)) {
            Value q = eval(idx->qubit.get());
            ensureQubitActive(q.qubit, idx->line, idx->column);
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
                throw BlochError(ErrorCategory::Runtime, indexExpr->line, indexExpr->column,
                                 "index must be numeric");
            switch (coll.type) {
                case Value::Type::BitArray:
                    if (idxi < 0 || idxi >= static_cast<int>(coll.bitArray.size()))
                        throw BlochError(ErrorCategory::Runtime, indexExpr->line, indexExpr->column,
                                         "index " + std::to_string(idxi) +
                                             " out of bounds for length " +
                                             std::to_string(coll.bitArray.size()));
                    return {Value::Type::Bit, 0, 0.0, coll.bitArray[idxi]};
                case Value::Type::IntArray:
                    if (idxi < 0 || idxi >= static_cast<int>(coll.intArray.size()))
                        throw BlochError(ErrorCategory::Runtime, indexExpr->line, indexExpr->column,
                                         "index " + std::to_string(idxi) +
                                             " out of bounds for length " +
                                             std::to_string(coll.intArray.size()));
                    return {Value::Type::Int, coll.intArray[idxi]};
                case Value::Type::FloatArray:
                    if (idxi < 0 || idxi >= static_cast<int>(coll.floatArray.size()))
                        throw BlochError(ErrorCategory::Runtime, indexExpr->line, indexExpr->column,
                                         "index " + std::to_string(idxi) +
                                             " out of bounds for length " +
                                             std::to_string(coll.floatArray.size()));
                    return {Value::Type::Float, 0, coll.floatArray[idxi]};
                case Value::Type::BooleanArray: {
                    if (idxi < 0 || idxi >= static_cast<int>(coll.boolArray.size()))
                        throw BlochError(ErrorCategory::Runtime, indexExpr->line, indexExpr->column,
                                         "index " + std::to_string(idxi) +
                                             " out of bounds for length " +
                                             std::to_string(coll.boolArray.size()));
                    Value v;
                    v.type = Value::Type::Boolean;
                    v.boolValue = coll.boolArray[idxi];
                    return v;
                }
                case Value::Type::StringArray:
                    if (idxi < 0 || idxi >= static_cast<int>(coll.stringArray.size()))
                        throw BlochError(ErrorCategory::Runtime, indexExpr->line, indexExpr->column,
                                         "index " + std::to_string(idxi) +
                                             " out of bounds for length " +
                                             std::to_string(coll.stringArray.size()));
                    return {Value::Type::String, 0, 0.0, 0, coll.stringArray[idxi]};
                case Value::Type::CharArray:
                    if (idxi < 0 || idxi >= static_cast<int>(coll.charArray.size()))
                        throw BlochError(ErrorCategory::Runtime, indexExpr->line, indexExpr->column,
                                         "index " + std::to_string(idxi) +
                                             " out of bounds for length " +
                                             std::to_string(coll.charArray.size()));
                    {
                        Value v;
                        v.type = Value::Type::Char;
                        v.charValue = coll.charArray[idxi];
                        return v;
                    }
                case Value::Type::QubitArray: {
                    if (idxi < 0 || idxi >= static_cast<int>(coll.qubitArray.size()))
                        throw BlochError(ErrorCategory::Runtime, indexExpr->line, indexExpr->column,
                                         "index " + std::to_string(idxi) +
                                             " out of bounds for length " +
                                             std::to_string(coll.qubitArray.size()));
                    Value v;
                    v.type = Value::Type::Qubit;
                    v.qubit = coll.qubitArray[idxi];
                    return v;
                }
                default:
                    throw BlochError(ErrorCategory::Runtime, indexExpr->line, indexExpr->column,
                                     "indexing requires an array value");
            }
        } else if (auto assignExpr = dynamic_cast<AssignmentExpression*>(e)) {
            Value v = eval(assignExpr->value.get());
            assign(assignExpr->name, v);
            return v;
        } else if (auto memAssign = dynamic_cast<MemberAssignmentExpression*>(e)) {
            Value obj = eval(memAssign->object.get());
            if (isNullReference(obj)) {
                throw BlochError(ErrorCategory::Runtime, memAssign->line, memAssign->column,
                                 "null reference");
            }
            Value rhs = eval(memAssign->value.get());
            if (obj.type == Value::Type::Object && obj.objectValue) {
                RuntimeField* instField =
                    obj.objectValue->cls
                        ? findInstanceField(obj.objectValue->cls, memAssign->member)
                        : nullptr;
                if (instField) {
                    if (instField->offset < obj.objectValue->fields.size())
                        obj.objectValue->fields[instField->offset] = rhs;
                } else {
                    auto [staticField, owner] =
                        obj.objectValue->cls
                            ? findStaticFieldWithOwner(obj.objectValue->cls, memAssign->member)
                            : std::pair<RuntimeField*, RuntimeClass*>{nullptr, nullptr};
                    if (staticField && owner && staticField->offset < owner->staticStorage.size())
                        owner->staticStorage[staticField->offset] = rhs;
                }
            } else if (obj.type == Value::Type::ClassRef && obj.classRef) {
                auto [field, owner] = findStaticFieldWithOwner(obj.classRef, memAssign->member);
                if (field && owner && field->offset < owner->staticStorage.size())
                    owner->staticStorage[field->offset] = rhs;
            }
            return rhs;
        } else if (auto aassign = dynamic_cast<ArrayAssignmentExpression*>(e)) {
            // Only support assigning into variable arrays for 1.0.0
            auto* var = dynamic_cast<VariableExpression*>(aassign->collection.get());
            if (!var)
                throw BlochError(ErrorCategory::Runtime, aassign->line, aassign->column,
                                 "assignment target must be a variable");
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
                throw BlochError(ErrorCategory::Runtime, aassign->line, aassign->column,
                                 "index must be numeric");
            Value rhs = eval(aassign->value.get());
            switch (arr.type) {
                case Value::Type::IntArray:
                    if (i < 0 || i >= static_cast<int>(arr.intArray.size()))
                        throw BlochError(ErrorCategory::Runtime, aassign->line, aassign->column,
                                         "index " + std::to_string(i) +
                                             " out of bounds for length " +
                                             std::to_string(arr.intArray.size()));
                    if (rhs.type == Value::Type::Int)
                        arr.intArray[i] = rhs.intValue;
                    else if (rhs.type == Value::Type::Bit)
                        arr.intArray[i] = rhs.bitValue;
                    else if (rhs.type == Value::Type::Float)
                        arr.intArray[i] = static_cast<int>(rhs.floatValue);
                    else
                        throw BlochError(ErrorCategory::Runtime, aassign->line, aassign->column,
                                         "type mismatch for int[] assignment");
                    break;
                case Value::Type::FloatArray:
                    if (i < 0 || i >= static_cast<int>(arr.floatArray.size()))
                        throw BlochError(ErrorCategory::Runtime, aassign->line, aassign->column,
                                         "index " + std::to_string(i) +
                                             " out of bounds for length " +
                                             std::to_string(arr.floatArray.size()));
                    if (rhs.type == Value::Type::Float)
                        arr.floatArray[i] = rhs.floatValue;
                    else if (rhs.type == Value::Type::Int)
                        arr.floatArray[i] = static_cast<double>(rhs.intValue);
                    else if (rhs.type == Value::Type::Bit)
                        arr.floatArray[i] = static_cast<double>(rhs.bitValue);
                    else
                        throw BlochError(ErrorCategory::Runtime, aassign->line, aassign->column,
                                         "type mismatch for float[] assignment");
                    break;
                case Value::Type::BitArray:
                    if (i < 0 || i >= static_cast<int>(arr.bitArray.size()))
                        throw BlochError(ErrorCategory::Runtime, aassign->line, aassign->column,
                                         "index " + std::to_string(i) +
                                             " out of bounds for length " +
                                             std::to_string(arr.bitArray.size()));
                    if (rhs.type == Value::Type::Bit)
                        arr.bitArray[i] = rhs.bitValue ? 1 : 0;
                    else if (rhs.type == Value::Type::Int)
                        arr.bitArray[i] = (rhs.intValue != 0) ? 1 : 0;
                    else
                        throw BlochError(ErrorCategory::Runtime, aassign->line, aassign->column,
                                         "type mismatch for bit[] assignment");
                    break;
                case Value::Type::BooleanArray:
                    if (i < 0 || i >= static_cast<int>(arr.boolArray.size()))
                        throw BlochError(ErrorCategory::Runtime, aassign->line, aassign->column,
                                         "index " + std::to_string(i) +
                                             " out of bounds for length " +
                                             std::to_string(arr.boolArray.size()));
                    if (rhs.type == Value::Type::Boolean)
                        arr.boolArray[i] = rhs.boolValue;
                    else if (rhs.type == Value::Type::Bit)
                        arr.boolArray[i] = rhs.bitValue != 0;
                    else
                        throw BlochError(ErrorCategory::Runtime, aassign->line, aassign->column,
                                         "type mismatch for boolean[] assignment");
                    break;
                case Value::Type::StringArray:
                    if (i < 0 || i >= static_cast<int>(arr.stringArray.size()))
                        throw BlochError(ErrorCategory::Runtime, aassign->line, aassign->column,
                                         "index " + std::to_string(i) +
                                             " out of bounds for length " +
                                             std::to_string(arr.stringArray.size()));
                    if (rhs.type == Value::Type::String)
                        arr.stringArray[i] = rhs.stringValue;
                    else
                        throw BlochError(ErrorCategory::Runtime, aassign->line, aassign->column,
                                         "type mismatch for string[] assignment");
                    break;
                case Value::Type::CharArray:
                    if (i < 0 || i >= static_cast<int>(arr.charArray.size()))
                        throw BlochError(ErrorCategory::Runtime, aassign->line, aassign->column,
                                         "index " + std::to_string(i) +
                                             " out of bounds for length " +
                                             std::to_string(arr.charArray.size()));
                    if (rhs.type == Value::Type::Char)
                        arr.charArray[i] = rhs.charValue;
                    else
                        throw BlochError(ErrorCategory::Runtime, aassign->line, aassign->column,
                                         "type mismatch for char[] assignment");
                    break;
                default:
                    throw BlochError(ErrorCategory::Runtime, aassign->line, aassign->column,
                                     "assignment into this array type is unsupported");
            }
            assign(var->name, arr);
            return arr;
        }
        return {};
    }

    int RuntimeEvaluator::allocateTrackedQubit(const std::string& name) {
        int idx = -1;
        if (!m_freeQubitIndices.empty()) {
            idx = m_freeQubitIndices.back();
            m_freeQubitIndices.pop_back();
            m_sim.reset(idx);
            unmarkMeasured(idx);
        } else {
            idx = m_sim.allocateQubit();
            m_qubits.push_back({name, false});
        }
        if (idx >= static_cast<int>(m_qubits.size()))
            m_qubits.resize(idx + 1);
        m_qubits[idx].name = name;
        m_qubits[idx].measured = false;
        if (idx >= static_cast<int>(m_lastMeasurement.size()))
            m_lastMeasurement.resize(idx + 1, -1);
        return idx;
    }

    void RuntimeEvaluator::markMeasured(int index) {
        if (index >= 0 && index < static_cast<int>(m_qubits.size()))
            m_qubits[index].measured = true;
    }

    void RuntimeEvaluator::releaseQubit(int index) {
        if (index < 0 || index >= static_cast<int>(m_qubits.size()))
            return;
        unmarkMeasured(index);
        m_qubits[index].name.clear();
        m_freeQubitIndices.push_back(index);
    }

    void RuntimeEvaluator::ensureQubitExists(int index, int line, int column) {
        if (index < 0 || index >= static_cast<int>(m_qubits.size()))
            throw BlochError(ErrorCategory::Runtime, line, column, "invalid qubit reference");
    }

    void RuntimeEvaluator::ensureQubitActive(int index, int line, int column) {
        ensureQubitExists(index, line, column);
        if (m_qubits[index].measured) {
            std::string label = m_qubits[index].name.empty()
                                    ? std::string("q[") + std::to_string(index) + "]"
                                    : m_qubits[index].name;
            throw BlochError(ErrorCategory::Runtime, line, column,
                             "qubit " + label + " has already been measured");
        }
    }

    void RuntimeEvaluator::unmarkMeasured(int index) {
        if (index >= 0 && index < static_cast<int>(m_qubits.size()))
            m_qubits[index].measured = false;
        if (index >= 0 && index < static_cast<int>(m_lastMeasurement.size()))
            m_lastMeasurement[index] = -1;
    }

    void RuntimeEvaluator::warnUnmeasured() const {
        for (const auto& q : m_qubits) {
            if (q.name.empty())
                continue;
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

    size_t RuntimeEvaluator::heapObjectCount() {
        std::lock_guard<std::mutex> lock(m_heapMutex);
        size_t count = 0;
        for (const auto& w : m_heap) {
            if (w.lock())
                ++count;
        }
        return count;
    }
}  // namespace bloch::runtime
