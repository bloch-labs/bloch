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

#include "bloch/compiler/semantics/semantic_analyser.hpp"

#include <algorithm>
#include <functional>
#include <iostream>
#include <limits>
#include <unordered_map>

#include "bloch/compiler/semantics/built_ins.hpp"

namespace bloch::compiler {

    using support::BlochError;
    using support::ErrorCategory;

    SemanticAnalyser::TypeInfo SemanticAnalyser::combine(ValueType prim, const std::string& cls) {
        TypeInfo t;
        t.value = prim;
        t.className = cls;
        return t;
    }

    bool SemanticAnalyser::typeEquals(const TypeInfo& a, const TypeInfo& b) {
        if (a.isTypeParam || b.isTypeParam)
            return a.isTypeParam && b.isTypeParam && a.className == b.className;
        if (a.value != b.value)
            return false;
        if (a.className != b.className)
            return false;
        if (a.typeArgs.size() != b.typeArgs.size())
            return false;
        for (size_t i = 0; i < a.typeArgs.size(); ++i) {
            if (!typeEquals(a.typeArgs[i], b.typeArgs[i]))
                return false;
        }
        return true;
    }

    std::string SemanticAnalyser::typeLabel(const TypeInfo& t) {
        if (t.isTypeParam)
            return t.className;
        auto isArray = [](const std::string& name) {
            return name.size() >= 2 && name.rfind("[]") == name.size() - 2;
        };
        if (isArray(t.className) && !t.typeArgs.empty()) {
            return typeLabel(t.typeArgs.front()) + "[]";
        }
        if (!t.className.empty()) {
            std::string res = t.className;
            if (!t.typeArgs.empty()) {
                res += "<";
                for (size_t i = 0; i < t.typeArgs.size(); ++i) {
                    if (i)
                        res += ",";
                    res += typeLabel(t.typeArgs[i]);
                }
                res += ">";
            }
            return res;
        }
        return typeToString(t.value);
    }

    namespace {
        std::string methodSignatureLabel(const std::string& name,
                                         const std::vector<SemanticAnalyser::TypeInfo>& params) {
            std::ostringstream oss;
            oss << name << "(";
            for (size_t i = 0; i < params.size(); ++i) {
                if (i)
                    oss << ",";
                oss << SemanticAnalyser::typeLabel(params[i]);
            }
            oss << ")";
            return oss.str();
        }

        bool paramTypesEqual(const std::vector<SemanticAnalyser::TypeInfo>& a,
                             const std::vector<SemanticAnalyser::TypeInfo>& b) {
            if (a.size() != b.size())
                return false;
            for (size_t i = 0; i < a.size(); ++i) {
                if (!SemanticAnalyser::typeEquals(a[i], b[i]))
                    return false;
            }
            return true;
        }

        auto numericPromotion = [](ValueType a, ValueType b) {
            if (a == ValueType::Float || b == ValueType::Float)
                return ValueType::Float;
            if (a == ValueType::Long || b == ValueType::Long)
                return ValueType::Long;
            if (a == ValueType::Int || b == ValueType::Int)
                return ValueType::Int;
            if (a == ValueType::Bit || b == ValueType::Bit)
                return ValueType::Bit;
            return ValueType::Unknown;
        };

        auto matchesPrimitive = [](ValueType expected, ValueType actual) {
            if (expected == ValueType::Unknown || actual == ValueType::Unknown)
                return true;
            if (expected == actual)
                return true;
            if (expected == ValueType::Long && actual == ValueType::Int)
                return true;  // widening
            return false;
        };

        bool isArrayTypeName(const std::string& name) {
            return name.size() >= 2 && name.rfind("[]") == name.size() - 2;
        }

        bool isArrayType(const SemanticAnalyser::TypeInfo& t) {
            return !t.className.empty() && isArrayTypeName(t.className);
        }

        bool isClassRefType(const SemanticAnalyser::TypeInfo& t) {
            return !t.className.empty() && !isArrayTypeName(t.className);
        }

        bool isNumericPrimitive(ValueType v) {
            return v == ValueType::Int || v == ValueType::Long || v == ValueType::Float;
        }

        bool isNumericType(const SemanticAnalyser::TypeInfo& t) {
            return t.className.empty() && isNumericPrimitive(t.value);
        }

        bool isBooleanLike(const SemanticAnalyser::TypeInfo& t) {
            return t.className.empty() &&
                   (t.value == ValueType::Boolean || t.value == ValueType::Bit);
        }

        bool isBitArrayType(const SemanticAnalyser::TypeInfo& t) {
            return isArrayType(t) && !t.typeArgs.empty() && t.typeArgs[0].className.empty() &&
                   t.typeArgs[0].value == ValueType::Bit;
        }
    }  // namespace

    SemanticAnalyser::TypeInfo SemanticAnalyser::typeFromAst(Type* typeNode) const {
        if (!typeNode)
            return combine(ValueType::Unknown, "");
        if (auto prim = dynamic_cast<PrimitiveType*>(typeNode))
            return combine(typeFromString(prim->name), "");
        if (auto named = dynamic_cast<NamedType*>(typeNode)) {
            std::string cls = named->nameParts.empty() ? "" : named->nameParts.back();
            if (!m_inClassRegistryBuild && named->hasTypeArgumentList &&
                named->typeArguments.empty()) {
                const ClassInfo* info = findClass(cls);
                if (info && info->typeParams.empty()) {
                    throw BlochError(ErrorCategory::Semantic, named->line, named->column,
                                     "type '" + cls + "' is not generic");
                }
                throw BlochError(ErrorCategory::Semantic, named->line, named->column,
                                 "cannot infer type arguments for '" + cls + "' in this context");
            }
            for (const auto& tp : m_currentTypeParams) {
                if (tp.name == cls) {
                    TypeInfo t = combine(ValueType::Unknown, cls);
                    t.isTypeParam = true;
                    return t;
                }
            }
            TypeInfo t = combine(ValueType::Unknown, cls);
            for (const auto& arg : named->typeArguments) {
                t.typeArgs.push_back(typeFromAst(arg.get()));
            }
            if (!m_inClassRegistryBuild)
                validateTypeApplication(t, named->line, named->column);
            return t;
        }
        if (dynamic_cast<VoidType*>(typeNode))
            return combine(ValueType::Void, "");
        if (auto arr = dynamic_cast<ArrayType*>(typeNode)) {
            auto elem = typeFromAst(arr->elementType.get());
            std::string base = elem.className.empty() ? typeToString(elem.value) : elem.className;
            TypeInfo t = combine(ValueType::Unknown, base + "[]");
            t.typeArgs.clear();
            t.typeArgs.push_back(elem);
            return t;
        }
        return combine(ValueType::Unknown, "");
    }

    const SemanticAnalyser::ClassInfo* SemanticAnalyser::findClass(const std::string& name) const {
        auto it = m_classes.find(name);
        if (it != m_classes.end())
            return &it->second;
        return nullptr;
    }

    SemanticAnalyser::TypeInfo SemanticAnalyser::substituteTypeParams(
        const TypeInfo& t, const std::vector<ClassInfo::TypeParamInfo>& params,
        const std::vector<TypeInfo>& args) const {
        if (t.isTypeParam) {
            for (size_t i = 0; i < params.size(); ++i) {
                if (params[i].name == t.className && i < args.size())
                    return args[i];
            }
        }
        TypeInfo out = t;
        out.typeArgs.clear();
        for (const auto& a : t.typeArgs)
            out.typeArgs.push_back(substituteTypeParams(a, params, args));
        auto isArray = [](const std::string& name) {
            return name.size() >= 2 && name.rfind("[]") == name.size() - 2;
        };
        if (isArray(out.className) && !out.typeArgs.empty()) {
            out.className = typeLabel(out.typeArgs.front()) + "[]";
        }
        return out;
    }

    std::vector<SemanticAnalyser::TypeInfo> SemanticAnalyser::substituteMany(
        const std::vector<TypeInfo>& types, const std::vector<ClassInfo::TypeParamInfo>& params,
        const std::vector<TypeInfo>& args) const {
        std::vector<TypeInfo> res;
        res.reserve(types.size());
        for (const auto& t : types) res.push_back(substituteTypeParams(t, params, args));
        return res;
    }

    void SemanticAnalyser::validateTypeApplication(const TypeInfo& t, int line, int column) const {
        if (t.className.empty())
            return;
        const ClassInfo* info = findClass(t.className);
        if (!info)
            return;
        if (info->typeParams.size() != t.typeArgs.size()) {
            throw BlochError(ErrorCategory::Semantic, line, column,
                             "type '" + t.className + "' expects " +
                                 std::to_string(info->typeParams.size()) + " type argument(s)");
        }
        for (size_t i = 0; i < info->typeParams.size() && i < t.typeArgs.size(); ++i) {
            const auto& bound = info->typeParams[i].bound;
            const auto& actual = t.typeArgs[i];
            if (!bound.className.empty()) {
                if (actual.isTypeParam)
                    continue;
                if (actual.className.empty()) {
                    throw BlochError(ErrorCategory::Semantic, line, column,
                                     "type argument '" + typeLabel(actual) +
                                         "' does not satisfy bound '" + typeLabel(bound) + "'");
                }
                if (actual.className != bound.className &&
                    !isSubclassOf(actual.className, bound.className)) {
                    throw BlochError(ErrorCategory::Semantic, line, column,
                                     "type argument '" + typeLabel(actual) +
                                         "' does not satisfy bound '" + typeLabel(bound) + "'");
                }
            }
        }
    }

    std::optional<SemanticAnalyser::TypeInfo> SemanticAnalyser::getTypeParamBound(
        const std::string& name) const {
        for (const auto& tp : m_currentTypeParams) {
            if (tp.name == name) {
                if (tp.bound.className.empty() && tp.bound.value == ValueType::Unknown &&
                    tp.bound.typeArgs.empty())
                    return std::nullopt;
                return tp.bound;
            }
        }
        return std::nullopt;
    }

    SemanticAnalyser::MethodInfo* SemanticAnalyser::findMethodInHierarchy(
        const TypeInfo& classType, const std::string& method,
        const std::vector<TypeInfo>* params) const {
        TypeInfo searchType = classType;
        if (classType.isTypeParam) {
            auto bound = getTypeParamBound(classType.className);
            if (!bound || bound->className.empty())
                return nullptr;
            searchType = *bound;
        }
        const ClassInfo* cur = findClass(searchType.className);
        if (!params) {
            while (cur) {
                auto mit = cur->methods.find(method);
                if (mit != cur->methods.end() && !mit->second.empty())
                    return const_cast<MethodInfo*>(&mit->second.front());
                if (cur->base.empty())
                    break;
                cur = findClass(cur->base);
            }
            return nullptr;
        }

        struct Candidate {
            MethodInfo* method = nullptr;
            int cost = 0;
        };
        std::unordered_set<std::string> hiddenSignatures;
        std::vector<Candidate> matches;
        while (cur) {
            auto mit = cur->methods.find(method);
            if (mit != cur->methods.end()) {
                for (auto& cand : mit->second) {
                    auto expected =
                        substituteMany(cand.paramTypes, cur->typeParams, searchType.typeArgs);
                    std::string signature = methodSignatureLabel(cand.name, expected);
                    if (hiddenSignatures.count(signature))
                        continue;
                    hiddenSignatures.insert(signature);
                    auto cost = paramsConversionCost(expected, *params);
                    if (!cost)
                        continue;
                    matches.push_back({const_cast<MethodInfo*>(&cand), *cost});
                }
            }
            if (cur->base.empty())
                break;
            cur = findClass(cur->base);
        }
        if (matches.empty())
            return nullptr;

        int bestCost = std::numeric_limits<int>::max();
        MethodInfo* best = nullptr;
        bool ambiguous = false;
        for (const auto& candidate : matches) {
            if (candidate.cost < bestCost) {
                bestCost = candidate.cost;
                best = candidate.method;
                ambiguous = false;
            } else if (candidate.cost == bestCost) {
                ambiguous = true;
            }
        }
        if (ambiguous)
            return nullptr;
        return best;
    }

    SemanticAnalyser::FieldInfo* SemanticAnalyser::findFieldInHierarchy(
        const TypeInfo& classType, const std::string& field) const {
        TypeInfo searchType = classType;
        if (classType.isTypeParam) {
            auto bound = getTypeParamBound(classType.className);
            if (!bound || bound->className.empty())
                return nullptr;
            searchType = *bound;
        }
        const ClassInfo* cur = findClass(searchType.className);
        while (cur) {
            auto fit = cur->fields.find(field);
            if (fit != cur->fields.end())
                return const_cast<FieldInfo*>(&fit->second);
            if (cur->base.empty())
                break;
            cur = findClass(cur->base);
        }
        return nullptr;
    }

    const SemanticAnalyser::FieldInfo* SemanticAnalyser::resolveField(const std::string& name,
                                                                      int line, int column) const {
        if (m_currentClass.empty())
            return nullptr;
        TypeInfo curType = combine(ValueType::Unknown, m_currentClass);
        FieldInfo* field = findFieldInHierarchy(curType, name);
        if (!field)
            return nullptr;
        if (!isAccessible(field->visibility, field->owner, m_currentClass)) {
            throw BlochError(ErrorCategory::Semantic, line, column,
                             "field '" + name + "' is not accessible here");
        }
        if (m_inStaticContext && !field->isStatic) {
            throw BlochError(ErrorCategory::Semantic, line, column,
                             "instance field '" + name +
                                 "' cannot be referenced in a static "
                                 "context");
        }
        return field;
    }

    bool SemanticAnalyser::isSubclassOf(const std::string& derived, const std::string& base) const {
        if (derived.empty() || base.empty())
            return false;
        const ClassInfo* cur = findClass(derived);
        while (cur) {
            if (cur->base == base)
                return true;
            cur = findClass(cur->base);
        }
        return false;
    }

    int SemanticAnalyser::inheritanceDistance(const std::string& derived,
                                              const std::string& base) const {
        if (derived.empty() || base.empty())
            return -1;
        if (derived == base)
            return 0;
        int distance = 0;
        const ClassInfo* cur = findClass(derived);
        while (cur && !cur->base.empty()) {
            ++distance;
            if (cur->base == base)
                return distance;
            cur = findClass(cur->base);
        }
        return -1;
    }

    bool SemanticAnalyser::isAssignableType(const TypeInfo& expected,
                                            const TypeInfo& actual) const {
        bool expectedIsArray = isArrayType(expected);
        bool expectedIsClassRef = isClassRefType(expected);

        if (actual.value == ValueType::Null) {
            return expectedIsClassRef;
        }

        if (expected.className.empty()) {
            if (expected.value == ValueType::Unknown || actual.value == ValueType::Unknown)
                return true;
            if (actual.className.empty())
                return matchesPrimitive(expected.value, actual.value);
            return false;
        }

        if (expected.isTypeParam) {
            if (actual.value != ValueType::Unknown && actual.className.empty())
                return false;  // primitives cannot satisfy type params
            if (actual.isTypeParam)
                return expected.className == actual.className;
            if (auto bound = getTypeParamBound(expected.className)) {
                if (!bound->className.empty())
                    return isAssignableType(*bound, actual);
            }
            return true;
        }

        if (expectedIsArray) {
            if (!isArrayType(actual))
                return false;
            return typeEquals(expected, actual);
        }

        if (actual.className.empty())
            return actual.value == ValueType::Unknown;
        if (actual.isTypeParam) {
            auto bound = getTypeParamBound(actual.className);
            if (!bound || bound->className.empty())
                return false;
            return isAssignableType(expected, *bound);
        }

        if (typeEquals(expected, actual))
            return true;

        if (actual.typeArgs.empty() && expected.typeArgs.empty()) {
            return actual.className == expected.className ||
                   isSubclassOf(actual.className, expected.className);
        }

        return false;
    }

    std::optional<int> SemanticAnalyser::conversionCost(const TypeInfo& expected,
                                                        const TypeInfo& actual) const {
        if (actual.value == ValueType::Null) {
            return isClassRefType(expected) ? std::optional<int>(3) : std::nullopt;
        }

        if (expected.className.empty()) {
            if (expected.value == ValueType::Unknown || actual.value == ValueType::Unknown)
                return 0;
            if (actual.className.empty()) {
                if (expected.value == actual.value)
                    return 0;
                if (expected.value == ValueType::Long && actual.value == ValueType::Int)
                    return 1;
            }
            return std::nullopt;
        }

        if (expected.isTypeParam) {
            if (actual.value != ValueType::Unknown && actual.className.empty())
                return std::nullopt;
            if (actual.isTypeParam)
                return expected.className == actual.className ? std::optional<int>(0)
                                                              : std::nullopt;
            if (auto bound = getTypeParamBound(expected.className)) {
                if (!bound->className.empty())
                    return conversionCost(*bound, actual);
            }
            return 1;
        }

        if (isArrayType(expected)) {
            if (!isArrayType(actual))
                return std::nullopt;
            return typeEquals(expected, actual) ? std::optional<int>(0) : std::nullopt;
        }

        if (actual.className.empty())
            return actual.value == ValueType::Unknown ? std::optional<int>(0) : std::nullopt;
        if (actual.isTypeParam) {
            auto bound = getTypeParamBound(actual.className);
            if (!bound || bound->className.empty())
                return std::nullopt;
            return conversionCost(expected, *bound);
        }

        if (typeEquals(expected, actual))
            return 0;

        if (actual.typeArgs.empty() && expected.typeArgs.empty()) {
            int distance = inheritanceDistance(actual.className, expected.className);
            if (distance >= 0)
                return distance;
        }

        return std::nullopt;
    }

    std::optional<int> SemanticAnalyser::paramsConversionCost(
        const std::vector<TypeInfo>& expected, const std::vector<TypeInfo>& actual) const {
        if (expected.size() != actual.size())
            return std::nullopt;

        int total = 0;
        for (size_t i = 0; i < expected.size(); ++i) {
            auto cost = conversionCost(expected[i], actual[i]);
            if (!cost)
                return std::nullopt;
            total += *cost;
        }
        return total;
    }

    bool SemanticAnalyser::paramsAssignable(const std::vector<TypeInfo>& expected,
                                            const std::vector<TypeInfo>& actual) const {
        return paramsConversionCost(expected, actual).has_value();
    }

    void SemanticAnalyser::validateOverrides(ClassInfo& info) {
        if (info.base.empty()) {
            for (auto& kv : info.methods) {
                for (auto& m : kv.second) {
                    if (m.isOverride) {
                        throw BlochError(ErrorCategory::Semantic, m.line, m.column,
                                         "'" + m.name + "' marked override but class has no base");
                    }
                    if (m.isStatic && m.isVirtual) {
                        throw BlochError(ErrorCategory::Semantic, m.line, m.column,
                                         "static method '" + m.name + "' cannot be virtual");
                    }
                }
            }
            return;
        }
        for (auto& kv : info.methods) {
            for (auto& m : kv.second) {
                if (m.isStatic && (m.isVirtual || m.isOverride)) {
                    throw BlochError(
                        ErrorCategory::Semantic, m.line, m.column,
                        "static method '" + m.name + "' cannot be declared virtual or override");
                }
                const MethodInfo* baseMethod = findMethodInHierarchy(
                    combine(ValueType::Unknown, info.base), m.name, &m.paramTypes);
                if (m.isOverride) {
                    if (!baseMethod) {
                        throw BlochError(
                            ErrorCategory::Semantic, m.line, m.column,
                            "'" + m.name + "' marked override but base method not found");
                    }
                    if (!baseMethod->isVirtual) {
                        throw BlochError(ErrorCategory::Semantic, m.line, m.column,
                                         "'" + m.name + "' overrides a non-virtual base method");
                    }
                    if (baseMethod->isStatic) {
                        throw BlochError(ErrorCategory::Semantic, m.line, m.column,
                                         "'" + m.name + "' cannot override a static base method");
                    }
                    if (!paramTypesEqual(baseMethod->paramTypes, m.paramTypes)) {
                        throw BlochError(ErrorCategory::Semantic, m.line, m.column,
                                         "parameter mismatch overriding '" + m.name + "'");
                    }
                    if (!typeEquals(baseMethod->returnType, m.returnType)) {
                        throw BlochError(ErrorCategory::Semantic, m.line, m.column,
                                         "return type mismatch overriding '" + m.name + "'");
                    }
                }
                if (m.isVirtual && m.isStatic) {
                    throw BlochError(ErrorCategory::Semantic, m.line, m.column,
                                     "static method '" + m.name + "' cannot be virtual");
                }
            }
        }
    }

    void SemanticAnalyser::validateAbstractness(ClassInfo& info) {
        std::vector<std::string> required;
        if (!info.base.empty()) {
            const ClassInfo* base = findClass(info.base);
            if (base)
                required.insert(required.end(), base->abstractMethods.begin(),
                                base->abstractMethods.end());
        }
        for (auto& kv : info.methods) {
            for (auto& m : kv.second) {
                if (m.isVirtual && !m.hasBody) {
                    required.push_back(m.signature);
                }
                if (m.hasBody) {
                    auto it = std::find(required.begin(), required.end(), m.signature);
                    if (it != required.end()) {
                        const MethodInfo* baseMethod = findMethodInHierarchy(
                            combine(ValueType::Unknown, info.base), m.name, &m.paramTypes);
                        if (baseMethod) {
                            if (m.isStatic) {
                                throw BlochError(ErrorCategory::Semantic, m.line, m.column,
                                                 "static method '" + m.name +
                                                     "' cannot implement abstract base method");
                            }
                            if (!paramTypesEqual(m.paramTypes, baseMethod->paramTypes) ||
                                !typeEquals(baseMethod->returnType, m.returnType)) {
                                throw BlochError(ErrorCategory::Semantic, m.line, m.column,
                                                 "implementation of abstract method '" + m.name +
                                                     "' has incompatible signature");
                            }
                        }
                        required.erase(it);
                    }
                }
            }
        }
        info.abstractMethods = required;
        if (!info.abstractMethods.empty())
            info.isAbstract = true;
    }

    bool SemanticAnalyser::isAccessible(Visibility visibility, const std::string& owner,
                                        const std::string& accessor) const {
        switch (visibility) {
            case Visibility::Public:
                return true;
            case Visibility::Private:
                return owner == accessor;
            case Visibility::Protected:
                return !accessor.empty() && (accessor == owner || isSubclassOf(accessor, owner));
            default:
                return false;
        }
    }

    bool SemanticAnalyser::isTypeReference(Expression* expr) const {
        if (auto var = dynamic_cast<VariableExpression*>(expr)) {
            return m_symbols.isTypeName(var->name);
        }
        return false;
    }

    bool SemanticAnalyser::isThisReference(Expression* expr) const {
        if (dynamic_cast<ThisExpression*>(expr))
            return true;
        if (auto var = dynamic_cast<VariableExpression*>(expr))
            return var->name == "this";
        return false;
    }

    bool SemanticAnalyser::isSuperConstructorCall(Statement* stmt) const {
        auto exprStmt = dynamic_cast<ExpressionStatement*>(stmt);
        if (!exprStmt)
            return false;
        auto call = dynamic_cast<CallExpression*>(exprStmt->expression.get());
        if (!call)
            return false;
        return dynamic_cast<SuperExpression*>(call->callee.get()) != nullptr;
    }

    std::unique_ptr<Type> SemanticAnalyser::typeFromTypeInfo(const TypeInfo& typeInfo) const {
        if (typeInfo.isTypeParam) {
            auto named = std::make_unique<NamedType>(std::vector<std::string>{typeInfo.className});
            return named;
        }
        if (isArrayType(typeInfo) && !typeInfo.typeArgs.empty()) {
            auto elementType = typeFromTypeInfo(typeInfo.typeArgs.front());
            if (!elementType)
                return nullptr;
            return std::make_unique<ArrayType>(std::move(elementType));
        }
        if (!typeInfo.className.empty()) {
            auto named = std::make_unique<NamedType>(std::vector<std::string>{typeInfo.className});
            for (const auto& arg : typeInfo.typeArgs) {
                auto argType = typeFromTypeInfo(arg);
                if (!argType)
                    return nullptr;
                named->typeArguments.push_back(std::move(argType));
            }
            return named;
        }
        switch (typeInfo.value) {
            case ValueType::Int:
                return std::make_unique<PrimitiveType>("int");
            case ValueType::Long:
                return std::make_unique<PrimitiveType>("long");
            case ValueType::Float:
                return std::make_unique<PrimitiveType>("float");
            case ValueType::Bit:
                return std::make_unique<PrimitiveType>("bit");
            case ValueType::Boolean:
                return std::make_unique<PrimitiveType>("boolean");
            case ValueType::String:
                return std::make_unique<PrimitiveType>("string");
            case ValueType::Char:
                return std::make_unique<PrimitiveType>("char");
            case ValueType::Qubit:
                return std::make_unique<PrimitiveType>("qubit");
            case ValueType::Void:
                return std::make_unique<VoidType>();
            default:
                return nullptr;
        }
    }

    void SemanticAnalyser::inferDiamondTypeArguments(Expression* initializer,
                                                     const TypeInfo& expectedType, int line,
                                                     int column) {
        auto* newExpr = dynamic_cast<NewExpression*>(initializer);
        if (!newExpr)
            return;
        auto* named = dynamic_cast<NamedType*>(newExpr->classType.get());
        if (!named || !named->hasTypeArgumentList || !named->typeArguments.empty() ||
            named->nameParts.empty())
            return;

        const std::string& className = named->nameParts.back();
        const ClassInfo* info = findClass(className);
        if (!info)
            return;
        if (info->typeParams.empty()) {
            throw BlochError(ErrorCategory::Semantic, line, column,
                             "type '" + className + "' is not generic");
        }
        if (expectedType.className.empty() || expectedType.className != className ||
            expectedType.typeArgs.empty()) {
            throw BlochError(
                ErrorCategory::Semantic, line, column,
                "cannot infer type arguments for '" + className + "' from assignment target");
        }
        if (info->typeParams.size() != expectedType.typeArgs.size()) {
            throw BlochError(
                ErrorCategory::Semantic, line, column,
                "cannot infer type arguments for '" + className + "' from assignment target");
        }

        for (const auto& typeArg : expectedType.typeArgs) {
            auto inferred = typeFromTypeInfo(typeArg);
            if (!inferred) {
                throw BlochError(ErrorCategory::Semantic, line, column,
                                 "cannot infer type arguments for '" + className + "'");
            }
            named->typeArguments.push_back(std::move(inferred));
        }
    }

    void SemanticAnalyser::validateTypedInitializer(const std::string& name, Type* declaredType,
                                                    Expression* initializer, int line, int column) {
        if (!declaredType || !initializer)
            return;

        TypeInfo targetInfo = typeFromAst(declaredType);
        inferDiamondTypeArguments(initializer, targetInfo, line, column);
        TypeInfo initInfo = inferTypeInfo(initializer);

        if (auto arr = dynamic_cast<ArrayType*>(declaredType)) {
            if (auto elem = dynamic_cast<PrimitiveType*>(arr->elementType.get())) {
                if (elem->name == "qubit") {
                    throw BlochError(ErrorCategory::Semantic, line, column,
                                     "qubit[] cannot be initialised");
                }
            }
            if (initInfo.value == ValueType::Null) {
                throw BlochError(ErrorCategory::Semantic, line, column,
                                 "initialiser for '" + name + "' cannot be null");
            }
        }

        if (auto call = dynamic_cast<CallExpression*>(initializer)) {
            if (auto callee = dynamic_cast<VariableExpression*>(call->callee.get())) {
                if (returnsVoid(callee->name)) {
                    throw BlochError(ErrorCategory::Semantic, line, column,
                                     "cannot assign result of 'void' function");
                }
            }
        }

        // Validate initializer expression first so cast errors surface before type checks.
        initializer->accept(*this);

        if (auto primType = targetInfo.value; primType != ValueType::Unknown) {
            ValueType initT = initInfo.value;
            if (!matchesPrimitive(primType, initT)) {
                if (primType == ValueType::Bit) {
                    if (auto lit = dynamic_cast<LiteralExpression*>(initializer)) {
                        if (lit->literalType == "int") {
                            throw BlochError(ErrorCategory::Semantic, line, column,
                                             "bit literals must be 0b or 1b");
                        }
                    }
                } else if (primType == ValueType::Float) {
                    if (auto lit = dynamic_cast<LiteralExpression*>(initializer)) {
                        if (lit->literalType == "int") {
                            throw BlochError(ErrorCategory::Semantic, line, column,
                                             "float literals must end with 'f'");
                        }
                    }
                }
                throw BlochError(ErrorCategory::Semantic, line, column,
                                 "initialiser for '" + name + "' expected '" +
                                     typeToString(primType) + "' but got '" + typeToString(initT) +
                                     "'");
            }
        } else if (!targetInfo.className.empty()) {
            bool targetIsArray =
                targetInfo.className.size() >= 2 &&
                targetInfo.className.rfind("[]") == targetInfo.className.size() - 2;
            if (initInfo.value == ValueType::Null) {
                if (targetIsArray) {
                    throw BlochError(ErrorCategory::Semantic, line, column,
                                     "initialiser for '" + name + "' cannot be null");
                }
            } else if (!isAssignableType(targetInfo, initInfo) &&
                       initInfo.value != ValueType::Unknown) {
                throw BlochError(
                    ErrorCategory::Semantic, line, column,
                    "initialiser for '" + name + "' expected '" + typeLabel(targetInfo) + "'");
            }
        }
    }

    void SemanticAnalyser::recordFinalFieldAssignment(const FieldInfo& field,
                                                      const std::string& fieldName, int line,
                                                      int column) {
        if (!field.isFinal)
            return;

        bool isConstructorFieldWrite = m_inConstructor && !field.isStatic;
        bool allowed = isConstructorFieldWrite && m_constructorFinalAssignmentDepth == 0;
        if (!allowed) {
            if (isConstructorFieldWrite && m_constructorFinalAssignmentDepth > 0) {
                throw BlochError(ErrorCategory::Semantic, line, column,
                                 "final field '" + fieldName +
                                     "' must be assigned as a top-level constructor statement");
            }
            throw BlochError(ErrorCategory::Semantic, line, column,
                             "cannot assign to final field '" + fieldName + "'");
        }
        if (field.owner != m_currentClass) {
            throw BlochError(ErrorCategory::Semantic, line, column,
                             "cannot assign inherited final field '" + fieldName + "'");
        }
        if (field.hasInitializer) {
            throw BlochError(
                ErrorCategory::Semantic, line, column,
                "cannot reassign final field '" + fieldName + "' with a declaration initialiser");
        }

        std::string key = field.owner + "::" + fieldName;
        int& assignmentCount = m_constructorFinalAssignments[key];
        assignmentCount += 1;
        if (assignmentCount > 1) {
            throw BlochError(
                ErrorCategory::Semantic, line, column,
                "final field '" + fieldName + "' may only be assigned once in a constructor");
        }
    }

    void SemanticAnalyser::buildClassRegistry(Program& program) {
        m_classes.clear();
        m_inClassRegistryBuild = true;

        bool hasExplicitObjectClass = false;
        for (const auto& clsNode : program.classes) {
            if (clsNode && clsNode->name == "Object") {
                hasExplicitObjectClass = true;
                break;
            }
        }
        if (!hasExplicitObjectClass) {
            ClassInfo rootObject;
            rootObject.name = "Object";
            rootObject.line = 0;
            rootObject.column = 0;
            rootObject.isStatic = false;
            rootObject.isAbstract = false;
            MethodInfo ctorInfo;
            ctorInfo.visibility = Visibility::Public;
            ctorInfo.hasBody = true;
            ctorInfo.isDefault = true;
            ctorInfo.owner = rootObject.name;
            ctorInfo.returnType = combine(ValueType::Unknown, rootObject.name);
            rootObject.constructors.push_back(std::move(ctorInfo));
            m_classes[rootObject.name] = std::move(rootObject);
        }

        for (auto& clsNode : program.classes) {
            if (!clsNode)
                continue;
            if (m_classes.count(clsNode->name)) {
                throw BlochError(ErrorCategory::Semantic, clsNode->line, clsNode->column,
                                 "class '" + clsNode->name + "' already declared");
            }
            ClassInfo info;
            info.name = clsNode->name;
            info.line = clsNode->line;
            info.column = clsNode->column;
            info.isStatic = clsNode->isStatic;
            info.isAbstract = clsNode->isAbstract;
            bool hasExplicitBase = false;
            if (clsNode->baseType) {
                if (auto named = dynamic_cast<NamedType*>(clsNode->baseType.get())) {
                    if (!named->nameParts.empty()) {
                        info.base = named->nameParts.back();
                        hasExplicitBase = true;
                    }
                }
            } else if (!clsNode->baseName.empty()) {
                info.base = clsNode->baseName.back();
                hasExplicitBase = true;
            } else if (!info.isStatic && info.name != "Object") {
                info.base = "Object";
            }
            if (info.name == "Object") {
                if (hasExplicitBase) {
                    throw BlochError(ErrorCategory::Semantic, info.line, info.column,
                                     "class 'Object' cannot declare a base class");
                }
                info.base.clear();
            }
            // Type parameters
            m_currentTypeParams.clear();
            for (auto& tp : clsNode->typeParameters) {
                ClassInfo::TypeParamInfo pi;
                pi.name = tp->name;
                if (tp->bound)
                    pi.bound = typeFromAst(tp->bound.get());
                m_currentTypeParams.push_back(pi);
                info.typeParams.push_back(pi);
            }
            if (info.name == "Object" && !info.typeParams.empty()) {
                throw BlochError(ErrorCategory::Semantic, info.line, info.column,
                                 "class 'Object' cannot declare type parameters");
            }
            for (auto& member : clsNode->members) {
                if (!member)
                    continue;
                if (auto field = dynamic_cast<FieldDeclaration*>(member.get())) {
                    if (info.fields.count(field->name)) {
                        throw BlochError(
                            ErrorCategory::Semantic, field->line, field->column,
                            "duplicate field '" + field->name + "' in class '" + info.name + "'");
                    }
                    if (info.isStatic && !field->isStatic) {
                        throw BlochError(
                            ErrorCategory::Semantic, field->line, field->column,
                            "static class '" + info.name + "' cannot declare instance fields");
                    }
                    if (field->isFinal && field->isStatic && !field->initializer) {
                        throw BlochError(
                            ErrorCategory::Semantic, field->line, field->column,
                            "final static field '" + field->name + "' must be initialised");
                    }
                    FieldInfo f;
                    f.visibility = field->visibility;
                    f.isStatic = field->isStatic;
                    f.isFinal = field->isFinal;
                    f.hasInitializer = field->initializer != nullptr;
                    f.isTracked = field->isTracked;
                    f.type = typeFromAst(field->fieldType.get());
                    f.owner = info.name;
                    f.line = field->line;
                    f.column = field->column;
                    info.fields[field->name] = f;
                } else if (auto method = dynamic_cast<MethodDeclaration*>(member.get())) {
                    if (info.isStatic && !method->isStatic) {
                        throw BlochError(
                            ErrorCategory::Semantic, method->line, method->column,
                            "static class '" + info.name + "' cannot declare instance methods");
                    }
                    MethodInfo m;
                    m.name = method->name;
                    m.visibility = method->visibility;
                    m.isStatic = method->isStatic;
                    m.isVirtual = method->isVirtual;
                    m.isOverride = method->isOverride;
                    m.hasBody = method->body != nullptr;
                    m.returnType = typeFromAst(method->returnType.get());
                    m.owner = info.name;
                    m.line = method->line;
                    m.column = method->column;
                    for (auto& p : method->params)
                        m.paramTypes.push_back(typeFromAst(p->type.get()));
                    m.signature = methodSignatureLabel(method->name, m.paramTypes);
                    if (info.methodSignatures.count(m.signature)) {
                        throw BlochError(
                            ErrorCategory::Semantic, method->line, method->column,
                            "duplicate method '" + m.signature + "' in class '" + info.name + "'");
                    }
                    info.methodSignatures.insert(m.signature);
                    if (m.isStatic && (m.isVirtual || m.isOverride)) {
                        throw BlochError(
                            ErrorCategory::Semantic, method->line, method->column,
                            "static method '" + method->name + "' cannot be virtual or override");
                    }
                    info.methods[method->name].push_back(m);
                    if (method->isVirtual && !m.hasBody)
                        info.abstractMethods.push_back(m.signature);
                } else if (auto ctor = dynamic_cast<ConstructorDeclaration*>(member.get())) {
                    if (info.isStatic) {
                        throw BlochError(
                            ErrorCategory::Semantic, ctor->line, ctor->column,
                            "static class '" + info.name + "' cannot declare constructors");
                    }
                    MethodInfo ctorInfo;
                    ctorInfo.visibility = ctor->visibility;
                    ctorInfo.hasBody = ctor->body != nullptr;
                    ctorInfo.isDefault = ctor->isDefault;
                    ctorInfo.owner = info.name;
                    ctorInfo.line = ctor->line;
                    ctorInfo.column = ctor->column;
                    for (auto& p : ctor->params)
                        ctorInfo.paramTypes.push_back(typeFromAst(p->type.get()));
                    ctorInfo.returnType = combine(ValueType::Unknown, info.name);
                    info.constructors.push_back(ctorInfo);
                    info.ctorDecls.push_back(ctor);
                } else if (auto dtor = dynamic_cast<DestructorDeclaration*>(member.get())) {
                    if (info.isStatic) {
                        throw BlochError(
                            ErrorCategory::Semantic, dtor->line, dtor->column,
                            "static class '" + info.name + "' cannot declare destructors");
                    }
                    if (info.hasUserDestructor) {
                        throw BlochError(
                            ErrorCategory::Semantic, dtor->line, dtor->column,
                            "class '" + info.name + "' cannot declare multiple destructors");
                    }
                    info.hasDestructor = true;
                    info.hasUserDestructor = true;
                }
            }
            if (!info.isStatic && info.constructors.empty()) {
                throw BlochError(ErrorCategory::Semantic, info.line, info.column,
                                 "class '" + info.name + "' must declare a constructor");
            }
            m_classes[info.name] = std::move(info);
        }

        for (auto& [name, info] : m_classes) {
            if (!info.base.empty() && !m_classes.count(info.base)) {
                throw BlochError(ErrorCategory::Semantic, 0, 0,
                                 "base class '" + info.base + "' not found for '" + name + "'");
            }
        }

        // Validate default constructors (parameter-field alignment)
        for (auto& [name, info] : m_classes) {
            for (auto* ctor : info.ctorDecls) {
                if (!ctor || !ctor->isDefault)
                    continue;
                for (auto& p : ctor->params) {
                    auto it = info.fields.find(p->name);
                    if (it == info.fields.end()) {
                        throw BlochError(ErrorCategory::Semantic, p->line, p->column,
                                         "default constructor parameter '" + p->name +
                                             "' must match an instance field");
                    }
                    const FieldInfo& f = it->second;
                    if (f.isStatic) {
                        throw BlochError(ErrorCategory::Semantic, p->line, p->column,
                                         "default constructor parameter '" + p->name +
                                             "' cannot bind to static field");
                    }
                    if (f.isFinal && f.hasInitializer) {
                        throw BlochError(ErrorCategory::Semantic, p->line, p->column,
                                         "default constructor cannot bind final field '" + p->name +
                                             "' because it already has a declaration initialiser");
                    }
                    if (f.type.value == ValueType::Qubit ||
                        (!f.type.className.empty() && f.type.value == ValueType::Unknown &&
                         f.type.className == "qubit")) {
                        throw BlochError(ErrorCategory::Semantic, p->line, p->column,
                                         "default constructor cannot bind qubit fields");
                    }
                    TypeInfo pt = typeFromAst(p->type.get());
                    if (!f.type.className.empty()) {
                        if (pt.className != f.type.className) {
                            throw BlochError(ErrorCategory::Semantic, p->line, p->column,
                                             "default constructor parameter '" + p->name +
                                                 "' must match field type '" + f.type.className +
                                                 "'");
                        }
                    } else if (pt.value != f.type.value) {
                        throw BlochError(ErrorCategory::Semantic, p->line, p->column,
                                         "default constructor parameter '" + p->name +
                                             "' must match field type '" +
                                             typeToString(f.type.value) + "'");
                    }
                }
            }
        }

        for (auto& [name, info] : m_classes) {
            std::unordered_set<std::string> seen;
            const ClassInfo* cur = &info;
            while (cur && !cur->base.empty()) {
                if (seen.count(cur->base)) {
                    throw BlochError(ErrorCategory::Semantic, info.line, info.column,
                                     "inheritance cycle involving '" + name + "'");
                }
                seen.insert(cur->base);
                cur = findClass(cur->base);
            }
        }

        std::unordered_set<std::string> validated;
        std::function<void(const std::string&)> validateClass = [&](const std::string& name) {
            if (validated.count(name))
                return;
            auto it = m_classes.find(name);
            if (it == m_classes.end())
                return;
            if (!it->second.base.empty())
                validateClass(it->second.base);
            validateOverrides(it->second);
            validateAbstractness(it->second);
            validated.insert(name);
        };
        for (auto& [name, _] : m_classes) {
            validateClass(name);
        }
        m_currentTypeParams.clear();
        m_inClassRegistryBuild = false;
    }

    void SemanticAnalyser::beginScope() {
        m_symbols.beginScope();
        m_typeStack.emplace_back();
    }
    void SemanticAnalyser::endScope() {
        m_symbols.endScope();
        if (!m_typeStack.empty())
            m_typeStack.pop_back();
    }

    void SemanticAnalyser::declare(const std::string& name, bool isFinal, const TypeInfo& type,
                                   bool isTypeName) {
        m_symbols.declare(name, isFinal, type.value, type.className, isTypeName);
        if (!isTypeName && !m_typeStack.empty())
            m_typeStack.back()[name] = type;
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

    std::vector<SemanticAnalyser::TypeInfo> SemanticAnalyser::getFunctionParamTypes(
        const std::string& name) const {
        auto it = m_functionInfo.find(name);
        if (it != m_functionInfo.end())
            return it->second.paramTypes;
        auto builtin = builtInGates.find(name);
        if (builtin != builtInGates.end()) {
            std::vector<TypeInfo> params;
            for (auto v : builtin->second.paramTypes) params.push_back(combine(v, ""));
            return params;
        }
        return {};
    }

    SemanticAnalyser::TypeInfo SemanticAnalyser::getVariableType(const std::string& name) const {
        for (auto it = m_typeStack.rbegin(); it != m_typeStack.rend(); ++it) {
            auto found = it->find(name);
            if (found != it->end())
                return found->second;
        }
        return combine(m_symbols.getType(name), m_symbols.getClassName(name));
    }

    std::string SemanticAnalyser::getVariableClassName(const std::string& name) const {
        return m_symbols.getClassName(name);
    }

    bool SemanticAnalyser::returnsVoid(const std::string& name) const {
        auto it = m_functionInfo.find(name);
        if (it != m_functionInfo.end())
            return it->second.returnType.value == ValueType::Void &&
                   it->second.returnType.className.empty();
        auto builtin = builtInGates.find(name);
        if (builtin != builtInGates.end())
            return builtin->second.returnType == ValueType::Void;
        return false;
    }

    SemanticAnalyser::TypeInfo SemanticAnalyser::inferTypeInfo(Expression* expr) const {
        if (!expr)
            return combine(ValueType::Unknown, "");
        if (dynamic_cast<NullLiteralExpression*>(expr))
            return combine(ValueType::Null, "");
        if (auto lit = dynamic_cast<LiteralExpression*>(expr))
            return combine(typeFromString(lit->literalType), "");
        if (auto var = dynamic_cast<VariableExpression*>(expr)) {
            TypeInfo local = getVariableType(var->name);
            if (local.value != ValueType::Unknown || !local.className.empty())
                return local;
            if (auto field = resolveField(var->name, var->line, var->column))
                return field->type;
            // If it's a known type name, treat it as a type reference (e.g., for static calls).
            if (m_symbols.isTypeName(var->name))
                return combine(ValueType::Unknown, var->name);
            return combine(ValueType::Unknown, "");
        }
        if (dynamic_cast<ThisExpression*>(expr))
            return combine(ValueType::Unknown, m_currentClass);
        if (auto par = dynamic_cast<ParenthesizedExpression*>(expr))
            return inferTypeInfo(par->expression.get());
        if (auto cast = dynamic_cast<CastExpression*>(expr))
            return typeFromAst(cast->targetType.get());
        if (dynamic_cast<MeasureExpression*>(expr))
            return combine(ValueType::Bit, "");
        if (dynamic_cast<SuperExpression*>(expr)) {
            const ClassInfo* cur = findClass(m_currentClass);
            if (cur && !cur->base.empty())
                return combine(ValueType::Unknown, cur->base);
            return combine(ValueType::Unknown, "");
        }
        if (auto call = dynamic_cast<CallExpression*>(expr)) {
            std::vector<TypeInfo> argTypes;
            for (auto& a : call->arguments) argTypes.push_back(inferTypeInfo(a.get()));
            if (auto callee = dynamic_cast<VariableExpression*>(call->callee.get())) {
                auto it = m_functionInfo.find(callee->name);
                if (it != m_functionInfo.end())
                    return it->second.returnType;
                auto bi = builtInGates.find(callee->name);
                if (bi != builtInGates.end())
                    return combine(bi->second.returnType, "");
                if (!m_currentClass.empty()) {
                    auto* method = findMethodInHierarchy(
                        combine(ValueType::Unknown, m_currentClass), callee->name, &argTypes);
                    if (method) {
                        if (!method->isStatic && m_inStaticContext)
                            return combine(ValueType::Unknown, "");
                        if (!isAccessible(method->visibility, method->owner, m_currentClass))
                            return combine(ValueType::Unknown, "");
                        return method->returnType;
                    }
                }
            } else if (auto mem = dynamic_cast<MemberAccessExpression*>(call->callee.get())) {
                auto obj = inferTypeInfo(mem->object.get());
                if (!obj.className.empty()) {
                    auto* method = findMethodInHierarchy(obj, mem->member, &argTypes);
                    if (method) {
                        TypeInfo ret = method->returnType;
                        const ClassInfo* cls = findClass(obj.className);
                        if (cls && !cls->typeParams.empty()) {
                            // First, substitute class-level type arguments directly.
                            ret = substituteTypeParams(ret, cls->typeParams, obj.typeArgs);
                            // Infer type arguments from actual call arguments (very simple: map
                            // type params to the corresponding actual argument types).
                            std::unordered_map<std::string, TypeInfo> binding;
                            std::function<void(const TypeInfo&, const TypeInfo&)> bindParams;
                            bindParams = [&](const TypeInfo& expected, const TypeInfo& actual) {
                                if (expected.isTypeParam) {
                                    binding[expected.className] = actual;
                                    return;
                                }
                                for (size_t i = 0;
                                     i < expected.typeArgs.size() && i < actual.typeArgs.size();
                                     ++i)
                                    bindParams(expected.typeArgs[i], actual.typeArgs[i]);
                            };
                            auto params = method->paramTypes;
                            for (size_t i = 0; i < params.size() && i < argTypes.size(); ++i)
                                bindParams(params[i], argTypes[i]);

                            std::function<TypeInfo(const TypeInfo&)> subst =
                                [&](const TypeInfo& t) {
                                    if (t.isTypeParam) {
                                        auto it = binding.find(t.className);
                                        if (it != binding.end())
                                            return it->second;
                                    }
                                    TypeInfo out = t;
                                    out.typeArgs.clear();
                                    for (const auto& a : t.typeArgs)
                                        out.typeArgs.push_back(subst(a));
                                    return out;
                                };
                            ret = subst(ret);
                        }
                        return ret;
                    }
                }
            }
            return combine(ValueType::Unknown, "");
        }
        if (auto bin = dynamic_cast<BinaryExpression*>(expr)) {
            auto lt = inferTypeInfo(bin->left.get());
            auto rt = inferTypeInfo(bin->right.get());
            if (bin->op == "==" || bin->op == "!=" || bin->op == "<" || bin->op == ">" ||
                bin->op == "<=" || bin->op == ">=" || bin->op == "&&" || bin->op == "||")
                return combine(ValueType::Boolean, "");
            if (bin->op == "+" && (lt.value == ValueType::String || rt.value == ValueType::String))
                return combine(ValueType::String, "");
            if (bin->op == "+") {
                ValueType promoted = numericPromotion(lt.value, rt.value);
                if (promoted == ValueType::Unknown)
                    return combine(ValueType::Unknown, "");
                return combine(promoted == ValueType::Bit ? ValueType::Int : promoted, "");
            }
            if (bin->op == "-" || bin->op == "*") {
                ValueType promoted = numericPromotion(lt.value, rt.value);
                if (promoted == ValueType::Unknown)
                    return combine(ValueType::Unknown, "");
                return combine(promoted == ValueType::Bit ? ValueType::Int : promoted, "");
            }
            if (bin->op == "/") {
                return combine(ValueType::Float, "");
            }
            if (bin->op == "%") {
                ValueType promoted = numericPromotion(lt.value, rt.value);
                if (promoted == ValueType::Unknown)
                    return combine(ValueType::Unknown, "");
                return combine(promoted == ValueType::Long ? ValueType::Long : ValueType::Int, "");
            }
            if (bin->op == "&" || bin->op == "|" || bin->op == "^") {
                if (lt.value == ValueType::Bit && rt.value == ValueType::Bit)
                    return combine(ValueType::Bit, "");
            }
            return combine(ValueType::Unknown, "");
        }
        if (auto un = dynamic_cast<UnaryExpression*>(expr)) {
            auto rt = inferTypeInfo(un->right.get());
            if (un->op == "-") {
                if (rt.value == ValueType::Float)
                    return combine(ValueType::Float, "");
                if (rt.value == ValueType::Long)
                    return combine(ValueType::Long, "");
                return combine(ValueType::Int, "");
            }
            if (un->op == "!")
                return combine(ValueType::Boolean, "");
            if (un->op == "~")
                return combine((rt.value == ValueType::Bit) ? ValueType::Bit : ValueType::Unknown,
                               "");
            return combine(ValueType::Unknown, "");
        }
        if (auto post = dynamic_cast<PostfixExpression*>(expr)) {
            if (auto v = dynamic_cast<VariableExpression*>(post->left.get())) {
                TypeInfo local = getVariableType(v->name);
                if (local.value != ValueType::Unknown || !local.className.empty())
                    return local;
                if (auto field = resolveField(v->name, v->line, v->column))
                    return field->type;
                return combine(ValueType::Unknown, "");
            }
            return combine(ValueType::Unknown, "");
        }
        if (auto mem = dynamic_cast<MemberAccessExpression*>(expr)) {
            auto obj = inferTypeInfo(mem->object.get());
            if (!obj.className.empty()) {
                TypeInfo searchType = obj;
                if (obj.isTypeParam) {
                    auto bound = getTypeParamBound(obj.className);
                    if (bound && !bound->className.empty())
                        searchType = *bound;
                }
                auto* field = findFieldInHierarchy(searchType, mem->member);
                if (field) {
                    if (!searchType.typeArgs.empty()) {
                        const ClassInfo* ci = findClass(searchType.className);
                        if (ci)
                            return substituteTypeParams(field->type, ci->typeParams,
                                                        searchType.typeArgs);
                    }
                    return field->type;
                }
                auto* method = findMethodInHierarchy(searchType, mem->member);
                if (method) {
                    if (!searchType.typeArgs.empty()) {
                        const ClassInfo* ci = findClass(searchType.className);
                        if (ci)
                            return substituteTypeParams(method->returnType, ci->typeParams,
                                                        searchType.typeArgs);
                    }
                    return method->returnType;
                }
            }
            return combine(ValueType::Unknown, "");
        }
        if (auto idx = dynamic_cast<IndexExpression*>(expr)) {
            auto collectionType = inferTypeInfo(idx->collection.get());
            if (isArrayType(collectionType) && !collectionType.typeArgs.empty())
                return collectionType.typeArgs.front();
            return combine(ValueType::Unknown, "");
        }
        if (auto newExpr = dynamic_cast<NewExpression*>(expr)) {
            return typeFromAst(newExpr->classType.get());
        }
        return combine(ValueType::Unknown, "");
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
            if (m_symbols.getType(var->name) != ValueType::Int)
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
        if (auto cast = dynamic_cast<CastExpression*>(expr)) {
            auto target = typeFromAst(cast->targetType.get());
            if (target.value == ValueType::Int)
                return evaluateConstInt(cast->expression.get());
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
                if (*right == 0)
                    throw BlochError(ErrorCategory::Semantic, bin->line, bin->column,
                                     "division by zero in constant integer expression");
                return *left / *right;
            }
            if (bin->op == "%") {
                if (*right == 0)
                    throw BlochError(ErrorCategory::Semantic, bin->line, bin->column,
                                     "modulo by zero in constant integer expression");
                return *left % *right;
            }
            return std::nullopt;
        }
        return std::nullopt;
    }

    void SemanticAnalyser::analyse(Program& program) {
        buildClassRegistry(program);
        beginScope();
        // Make class names visible as types/values (for static access).
        for (const auto& kv : m_classes) {
            declare(kv.first, true, combine(ValueType::Unknown, kv.first), true);
        }
        // Predeclare functions
        for (auto& fn : program.functions) {
            if (isFunctionDeclared(fn->name)) {
                throw BlochError(ErrorCategory::Semantic, fn->line, fn->column,
                                 "'" + fn->name + "' is already declared in this scope");
            }
            declareFunction(fn->name);
        }
        for (auto& cls : program.classes)
            if (cls)
                cls->accept(*this);
        for (auto& fn : program.functions) fn->accept(*this);
        for (auto& stmt : program.statements) stmt->accept(*this);
        endScope();
    }

    void SemanticAnalyser::visit(VariableDeclaration& node) {
        if (isDeclared(node.name)) {
            throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                             "'" + node.name + "' is already declared in this scope");
        }
        TypeInfo tinfo = typeFromAst(node.varType.get());
        if (tinfo.value == ValueType::Void) {
            throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                             "variables cannot have type 'void'");
        }
        for (const auto& ann : node.annotations) {
            if (ann && ann->name == "quantum") {
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "'@quantum' may annotate functions only");
            }
            if (ann && ann->name == "shots") {
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "'@shots(N)' can only decorate the main() function.");
            }
        }
        declare(node.name, node.isFinal, tinfo);
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
        if (node.isFinal && !node.initializer) {
            throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                             "final variable '" + node.name + "' must be initialised");
        }
        if (node.initializer)
            validateTypedInitializer(node.name, node.varType.get(), node.initializer.get(),
                                     node.line, node.column);
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
        bool trackNesting = m_inConstructor;
        if (trackNesting)
            m_constructorFinalAssignmentDepth += 1;
        beginScope();
        for (auto& stmt : node.statements) stmt->accept(*this);
        endScope();
        if (trackNesting)
            m_constructorFinalAssignmentDepth -= 1;
    }

    void SemanticAnalyser::visit(ExpressionStatement& node) {
        if (node.expression)
            node.expression->accept(*this);
    }

    void SemanticAnalyser::visit(ReturnStatement& node) {
        m_foundReturn = true;
        bool isVoid =
            (m_currentReturn.value == ValueType::Void && m_currentReturn.className.empty());
        if (node.value && isVoid) {
            throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                             "void function cannot return a value");
        }
        if (!node.value && !isVoid) {
            throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                             "Non-void function must return a value");
        }
        if (node.value) {
            inferDiamondTypeArguments(node.value.get(), m_currentReturn, node.line, node.column);
            auto actual = inferTypeInfo(node.value.get());
            if (!isVoid) {
                bool expectedIsArray =
                    m_currentReturn.className.size() >= 2 &&
                    m_currentReturn.className.rfind("[]") == m_currentReturn.className.size() - 2;
                if (actual.value == ValueType::Null) {
                    if (m_currentReturn.className.empty() || expectedIsArray) {
                        throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                         "return type mismatch");
                    }
                } else if (!m_currentReturn.className.empty()) {
                    if (!isAssignableType(m_currentReturn, actual)) {
                        throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                         "return type mismatch");
                    }
                } else if (!matchesPrimitive(m_currentReturn.value, actual.value)) {
                    throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                     "return type mismatch");
                }
            }
            node.value->accept(*this);
        }
    }

    void SemanticAnalyser::visit(IfStatement& node) {
        bool trackNesting = m_inConstructor;
        if (trackNesting)
            m_constructorFinalAssignmentDepth += 1;
        if (node.condition) {
            node.condition->accept(*this);
            auto condType = inferTypeInfo(node.condition.get());
            if (!isBooleanLike(condType)) {
                int line = node.condition->line > 0 ? node.condition->line : node.line;
                int col = node.condition->column > 0 ? node.condition->column : node.column;
                throw BlochError(ErrorCategory::Semantic, line, col,
                                 "if condition must be 'boolean' or 'bit'");
            }
        }
        if (node.thenBranch)
            node.thenBranch->accept(*this);
        if (node.elseBranch)
            node.elseBranch->accept(*this);
        if (trackNesting)
            m_constructorFinalAssignmentDepth -= 1;
    }

    void SemanticAnalyser::visit(TernaryStatement& node) {
        bool trackNesting = m_inConstructor;
        if (trackNesting)
            m_constructorFinalAssignmentDepth += 1;
        if (node.condition) {
            node.condition->accept(*this);
            auto condType = inferTypeInfo(node.condition.get());
            if (!isBooleanLike(condType)) {
                int line = node.condition->line > 0 ? node.condition->line : node.line;
                int col = node.condition->column > 0 ? node.condition->column : node.column;
                throw BlochError(ErrorCategory::Semantic, line, col,
                                 "conditional statement requires 'boolean' or 'bit' condition");
            }
        }
        if (node.thenBranch)
            node.thenBranch->accept(*this);
        if (node.elseBranch)
            node.elseBranch->accept(*this);
        if (trackNesting)
            m_constructorFinalAssignmentDepth -= 1;
    }

    void SemanticAnalyser::visit(ForStatement& node) {
        bool trackNesting = m_inConstructor;
        if (trackNesting)
            m_constructorFinalAssignmentDepth += 1;
        beginScope();
        if (node.initializer)
            node.initializer->accept(*this);
        if (node.condition) {
            node.condition->accept(*this);
            auto condType = inferTypeInfo(node.condition.get());
            if (!isBooleanLike(condType)) {
                int line = node.condition->line > 0 ? node.condition->line : node.line;
                int col = node.condition->column > 0 ? node.condition->column : node.column;
                throw BlochError(ErrorCategory::Semantic, line, col,
                                 "for-loop condition must be 'boolean' or 'bit'");
            }
        }
        if (node.increment)
            node.increment->accept(*this);
        if (node.body)
            node.body->accept(*this);
        endScope();
        if (trackNesting)
            m_constructorFinalAssignmentDepth -= 1;
    }

    void SemanticAnalyser::visit(WhileStatement& node) {
        bool trackNesting = m_inConstructor;
        if (trackNesting)
            m_constructorFinalAssignmentDepth += 1;
        if (node.condition) {
            node.condition->accept(*this);
            auto condType = inferTypeInfo(node.condition.get());
            if (!isBooleanLike(condType)) {
                int line = node.condition->line > 0 ? node.condition->line : node.line;
                int col = node.condition->column > 0 ? node.condition->column : node.column;
                throw BlochError(ErrorCategory::Semantic, line, col,
                                 "while condition must be 'boolean' or 'bit'");
            }
        }
        if (node.body)
            node.body->accept(*this);
        if (trackNesting)
            m_constructorFinalAssignmentDepth -= 1;
    }

    void SemanticAnalyser::visit(EchoStatement& node) {
        if (node.value)
            node.value->accept(*this);
    }

    void SemanticAnalyser::visit(ResetStatement& node) {
        if (node.target)
            node.target->accept(*this);
        auto tinfo = inferTypeInfo(node.target.get());
        if (tinfo.value != ValueType::Unknown && tinfo.value != ValueType::Qubit) {
            throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                             "reset target must be a 'qubit'");
        }
    }

    void SemanticAnalyser::visit(MeasureStatement& node) {
        if (node.qubit)
            node.qubit->accept(*this);
        auto tinfo = inferTypeInfo(node.qubit.get());
        bool isQubitArray = (!tinfo.className.empty() && tinfo.className == "qubit[]");
        bool isQubit = tinfo.value == ValueType::Qubit;
        if (tinfo.value != ValueType::Unknown && !isQubit && !isQubitArray) {
            throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                             "measure target must be a 'qubit' or 'qubit[]'");
        }
    }

    void SemanticAnalyser::visit(DestroyStatement& node) {
        if (node.target) {
            auto t = inferTypeInfo(node.target.get());
            if (t.className.empty() && t.value != ValueType::Null) {
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "'destroy' requires a class reference");
            }
            node.target->accept(*this);
        }
    }

    void SemanticAnalyser::visit(AssignmentStatement& node) {
        if (isDeclared(node.name)) {
            if (isFinal(node.name)) {
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "Cannot assign to final variable '" + node.name + "'");
            }
            if (node.value) {
                TypeInfo targetType = getVariableType(node.name);
                inferDiamondTypeArguments(node.value.get(), targetType, node.line, node.column);
                auto valType = inferTypeInfo(node.value.get());
                if (valType.value == ValueType::Null) {
                    bool targetIsArray =
                        targetType.className.size() >= 2 &&
                        targetType.className.rfind("[]") == targetType.className.size() - 2;
                    bool targetIsClass = !targetType.className.empty() && !targetIsArray;
                    if (!targetIsClass) {
                        throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                         "cannot assign null to '" + node.name + "'");
                    }
                } else if (valType.value != ValueType::Unknown &&
                           !isAssignableType(targetType, valType)) {
                    throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                     "assignment to '" + node.name + "' expects '" +
                                         typeLabel(targetType) + "'");
                }
                node.value->accept(*this);
            }
            return;
        }
        if (auto field = resolveField(node.name, node.line, node.column)) {
            recordFinalFieldAssignment(*field, node.name, node.line, node.column);
            if (node.value) {
                TypeInfo targetType = field->type;
                inferDiamondTypeArguments(node.value.get(), targetType, node.line, node.column);
                auto valType = inferTypeInfo(node.value.get());
                bool fieldIsArray =
                    targetType.className.size() >= 2 &&
                    targetType.className.rfind("[]") == targetType.className.size() - 2;
                if (valType.value == ValueType::Null) {
                    if (fieldIsArray || targetType.className.empty()) {
                        throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                         "cannot assign null to field '" + node.name + "'");
                    }
                }
                if (!targetType.className.empty() && valType.value != ValueType::Null &&
                    valType.value != ValueType::Unknown && !isAssignableType(targetType, valType)) {
                    throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                     "assignment to field '" + node.name + "' expects '" +
                                         typeLabel(targetType) + "'");
                } else if (field->type.value != ValueType::Unknown &&
                           valType.value != ValueType::Unknown &&
                           !matchesPrimitive(targetType.value, valType.value)) {
                    throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                     "assignment to field '" + node.name + "' expects '" +
                                         typeToString(targetType.value) + "'");
                }
                node.value->accept(*this);
            }
            return;
        }
        throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                         "Variable '" + node.name + "' not declared");
    }

    void SemanticAnalyser::visit(BinaryExpression& node) {
        if (node.left)
            node.left->accept(*this);
        if (node.right)
            node.right->accept(*this);
        auto lt = inferTypeInfo(node.left.get());
        auto rt = inferTypeInfo(node.right.get());
        auto isStringType = [](const TypeInfo& t) {
            return t.className.empty() && t.value == ValueType::String;
        };
        auto isBitType = [](const TypeInfo& t) {
            return t.className.empty() && t.value == ValueType::Bit;
        };
        auto errorWithTypes = [&](const std::string& message) {
            throw BlochError(ErrorCategory::Semantic, node.line, node.column, message);
        };
        if ((lt.value == ValueType::Null || rt.value == ValueType::Null) && node.op != "==" &&
            node.op != "!=") {
            throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                             "null can only be used in equality comparisons");
        }
        if (node.op == "==" || node.op == "!=") {
            bool leftNull = lt.value == ValueType::Null;
            bool rightNull = rt.value == ValueType::Null;
            if (leftNull && rightNull)
                return;
            if (leftNull || rightNull) {
                const TypeInfo& other = leftNull ? rt : lt;
                if (!isClassRefType(other)) {
                    throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                     "null comparison requires a class reference");
                }
                return;
            }
            if (isClassRefType(lt) || isClassRefType(rt)) {
                if (!(isClassRefType(lt) && isClassRefType(rt))) {
                    errorWithTypes("equality on references requires two class references");
                }
                return;
            }
            if (isArrayType(lt) || isArrayType(rt)) {
                errorWithTypes("equality on arrays is not supported");
            }
            bool lBool = isBooleanLike(lt);
            bool rBool = isBooleanLike(rt);
            if (lBool || rBool) {
                if (!(lBool && rBool)) {
                    errorWithTypes(
                        "equality requires boolean or bit operands when used with "
                        "boolean/bit");
                }
                return;
            }
            if (isNumericType(lt) && isNumericType(rt))
                return;
            if (lt.className.empty() && rt.className.empty() && lt.value == rt.value &&
                (lt.value == ValueType::String || lt.value == ValueType::Char)) {
                return;
            }
            errorWithTypes("operator '" + node.op + "' not supported for types '" + typeLabel(lt) +
                           "' and '" + typeLabel(rt) + "'");
        }

        if (node.op == "&&" || node.op == "||") {
            if (!(isBooleanLike(lt) && isBooleanLike(rt))) {
                errorWithTypes("logical operator '" + node.op +
                               "' requires boolean or bit operands");
            }
            return;
        }

        if (node.op == "&" || node.op == "|" || node.op == "^") {
            bool lBit = isBitType(lt);
            bool rBit = isBitType(rt);
            bool lBitArr = isBitArrayType(lt);
            bool rBitArr = isBitArrayType(rt);
            if (!((lBit || lBitArr) && (rBit || rBitArr))) {
                errorWithTypes("bitwise operator '" + node.op + "' requires bit or bit[] operands");
            }
            return;
        }

        if (node.op == "+" && (isStringType(lt) || isStringType(rt)))
            return;

        if (node.op == "+" || node.op == "-" || node.op == "*" || node.op == "/") {
            if (!(isNumericType(lt) && isNumericType(rt))) {
                errorWithTypes("operator '" + node.op +
                               "' requires numeric operands (int, long, float)");
            }
            return;
        }

        if (node.op == "%") {
            if (!(isNumericType(lt) && isNumericType(rt)) || lt.value == ValueType::Float ||
                rt.value == ValueType::Float) {
                errorWithTypes("operator '%' requires integer operands (int, long)");
            }
            return;
        }

        if (node.op == "<" || node.op == ">" || node.op == "<=" || node.op == ">=") {
            if (!(isNumericType(lt) && isNumericType(rt))) {
                errorWithTypes("operator '" + node.op +
                               "' requires numeric operands (int, long, float)");
            }
            return;
        }
    }

    void SemanticAnalyser::visit(UnaryExpression& node) {
        if (node.right)
            node.right->accept(*this);
        auto rt = inferTypeInfo(node.right.get());
        if (node.op == "!") {
            if (!isBooleanLike(rt)) {
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "logical '!' requires boolean or bit operand");
            }
            return;
        }
        if (node.op == "-") {
            if (!isNumericType(rt)) {
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "unary '-' requires numeric operand (int, long, float)");
            }
            return;
        }
        if (node.op == "~") {
            if (!(rt.className.empty() && rt.value == ValueType::Bit) && !isBitArrayType(rt)) {
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "bitwise '~' requires bit or bit[] operand");
            }
            return;
        }
    }

    void SemanticAnalyser::visit(CastExpression& node) {
        TypeInfo target = typeFromAst(node.targetType.get());
        auto source = inferTypeInfo(node.expression.get());
        auto isNumericNonChar = [](ValueType v) {
            return v == ValueType::Int || v == ValueType::Long || v == ValueType::Float ||
                   v == ValueType::Bit;
        };
        auto typeLabel = [](const TypeInfo& t) {
            if (!t.className.empty())
                return t.className;
            return typeToString(t.value);
        };
        auto reject = [&](const TypeInfo& from, const TypeInfo& to) {
            throw BlochError(
                ErrorCategory::Semantic, node.line, node.column,
                "Cannot explicitally cast from " + typeLabel(from) + " to " + typeLabel(to));
        };
        if (target.value == ValueType::Void || !target.className.empty() ||
            !isNumericNonChar(target.value)) {
            reject(source, target);
        }
        if (source.value != ValueType::Unknown) {
            if (!isNumericNonChar(source.value) || !source.className.empty()) {
                reject(source, target);
            }
        }
        if (node.expression)
            node.expression->accept(*this);
    }

    void SemanticAnalyser::visit(PostfixExpression& node) {
        if (auto var = dynamic_cast<VariableExpression*>(node.left.get())) {
            if (isDeclared(var->name)) {
                if (isFinal(var->name)) {
                    throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                     "Cannot modify final variable '" + var->name + "'");
                }
                TypeInfo t = getVariableType(var->name);
                if ((t.value != ValueType::Int && t.value != ValueType::Long) ||
                    !t.className.empty()) {
                    throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                     "Postfix operator '" + node.op +
                                         "' requires variable of type 'int' or 'long'");
                }
                return;
            }
            if (auto field = resolveField(var->name, node.line, node.column)) {
                if (field->isFinal) {
                    throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                     "Cannot modify final field '" + var->name + "'");
                }
                if (field->type.value != ValueType::Int && field->type.value != ValueType::Long) {
                    throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                     "Postfix operator '" + node.op +
                                         "' requires variable of type 'int' or 'long'");
                }
                return;
            }
            throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                             "Variable '" + var->name + "' not declared");
        } else if (node.left) {
            node.left->accept(*this);
            throw BlochError(
                ErrorCategory::Semantic, node.line, node.column,
                "Postfix operator '" + node.op + "' can only be applied to a variable");
        }
    }

    void SemanticAnalyser::visit(LiteralExpression&) {}
    void SemanticAnalyser::visit(NullLiteralExpression&) {}

    void SemanticAnalyser::visit(VariableExpression& node) {
        if (isDeclared(node.name) || isFunctionDeclared(node.name))
            return;
        if (resolveField(node.name, node.line, node.column))
            return;
        throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                         "Variable '" + node.name + "' not declared");
    }

    void SemanticAnalyser::visit(CallExpression& node) {
        std::vector<TypeInfo> actualTypes;
        actualTypes.reserve(node.arguments.size());
        for (auto& arg : node.arguments) actualTypes.push_back(inferTypeInfo(arg.get()));

        auto checkArgs = [&](const std::vector<TypeInfo>& params, const std::string& name, int line,
                             int column) {
            if (params.size() != node.arguments.size()) {
                throw BlochError(
                    ErrorCategory::Semantic, line, column,
                    "'" + name + "' expects " + std::to_string(params.size()) + " argument(s)");
            }
            for (size_t i = 0; i < node.arguments.size(); ++i) {
                auto expected = params[i];
                auto& arg = node.arguments[i];
                auto actual = actualTypes[i];
                bool expectedIsArray =
                    expected.className.size() >= 2 &&
                    expected.className.rfind("[]") == expected.className.size() - 2;
                if (!expected.className.empty()) {
                    if (expectedIsArray && actual.value == ValueType::Null) {
                        throw BlochError(ErrorCategory::Semantic, arg->line, arg->column,
                                         "argument #" + std::to_string(i + 1) + " to '" + name +
                                             "' expected '" + expected.className + "'");
                    }
                    if (expected.isTypeParam) {
                        if (actual.value != ValueType::Unknown && actual.className.empty()) {
                            throw BlochError(ErrorCategory::Semantic, arg->line, arg->column,
                                             "argument #" + std::to_string(i + 1) + " to '" + name +
                                                 "' expected type parameter '" +
                                                 expected.className + "'");
                        }
                        if (actual.isTypeParam) {
                            // Passing a type parameter to another type-parameter-typed parameter
                            // is always allowed; bound checking happens when the enclosing type is
                            // instantiated.
                            continue;
                        }
                        if (auto bound = getTypeParamBound(expected.className)) {
                            if (!bound->className.empty() && !actual.className.empty() &&
                                actual.className != bound->className &&
                                !isSubclassOf(actual.className, bound->className)) {
                                throw BlochError(ErrorCategory::Semantic, arg->line, arg->column,
                                                 "argument #" + std::to_string(i + 1) + " to '" +
                                                     name + "' must satisfy bound '" +
                                                     typeLabel(*bound) + "'");
                            }
                        }
                        continue;
                    }
                    if (actual.value == ValueType::Null)
                        continue;  // nullable class refs
                    if (actual.className.empty()) {
                        if (actual.value == ValueType::Unknown)
                            continue;
                        throw BlochError(ErrorCategory::Semantic, arg->line, arg->column,
                                         "argument #" + std::to_string(i + 1) + " to '" + name +
                                             "' expected '" + typeLabel(expected) + "'");
                    }
                    if (!isAssignableType(expected, actual)) {
                        throw BlochError(ErrorCategory::Semantic, arg->line, arg->column,
                                         "argument #" + std::to_string(i + 1) + " to '" + name +
                                             "' expected '" + typeLabel(expected) + "'");
                    }
                } else if (expected.value != ValueType::Unknown &&
                           actual.value != ValueType::Unknown &&
                           !matchesPrimitive(expected.value, actual.value)) {
                    throw BlochError(ErrorCategory::Semantic, arg->line, arg->column,
                                     "argument #" + std::to_string(i + 1) + " to '" + name +
                                         "' expected '" + typeToString(expected.value) + "'");
                }
            }
        };

        if (auto var = dynamic_cast<VariableExpression*>(node.callee.get())) {
            MethodInfo* methodInfo = nullptr;
            if (!isDeclared(var->name) && !isFunctionDeclared(var->name)) {
                if (!m_currentClass.empty())
                    methodInfo = findMethodInHierarchy(combine(ValueType::Unknown, m_currentClass),
                                                       var->name, &actualTypes);
                if (methodInfo) {
                    if (!isAccessible(methodInfo->visibility, methodInfo->owner, m_currentClass)) {
                        throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                         "method '" + var->name + "' is not accessible here");
                    }
                    if (!methodInfo->isStatic && m_inStaticContext) {
                        throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                         "instance method '" + var->name +
                                             "' cannot be called in a static context");
                    }
                } else {
                    if (resolveField(var->name, var->line, var->column)) {
                        throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                         "'" + var->name + "' is a field and cannot be called");
                    }
                    throw BlochError(ErrorCategory::Semantic, var->line, var->column,
                                     "Variable '" + var->name + "' not declared");
                }
            }
            if (methodInfo) {
                checkArgs(methodInfo->paramTypes, var->name, node.line, node.column);
            } else {
                size_t expected = getFunctionParamCount(var->name);
                if (expected != node.arguments.size()) {
                    throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                     "Function '" + var->name + "' expects " +
                                         std::to_string(expected) + " argument(s)");
                }
                auto types = getFunctionParamTypes(var->name);
                checkArgs(types, var->name, node.line, node.column);
            }
        } else if (auto member = dynamic_cast<MemberAccessExpression*>(node.callee.get())) {
            if (member->object)
                member->object->accept(*this);
            auto objType = inferTypeInfo(member->object.get());
            if (objType.className.empty()) {
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "member call requires a class reference");
            }
            TypeInfo searchType = objType;
            if (objType.isTypeParam) {
                auto bound = getTypeParamBound(objType.className);
                if (!bound || bound->className.empty()) {
                    throw BlochError(
                        ErrorCategory::Semantic, node.line, node.column,
                        "type parameter '" + objType.className + "' is not bound to a class type");
                }
                searchType = *bound;
            }
            const ClassInfo* cls = findClass(searchType.className);
            if (!cls) {
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "class '" + objType.className + "' not found");
            }
            MethodInfo* method = findMethodInHierarchy(searchType, member->member, &actualTypes);
            if (!method) {
                if (findFieldInHierarchy(objType, member->member)) {
                    throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                     "'" + member->member + "' is a field and cannot be called");
                }
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "method '" + member->member + "' not found on class '" +
                                     objType.className + "'");
            }
            if (!isAccessible(method->visibility, method->owner, m_currentClass)) {
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "member '" + member->member + "' is not accessible here");
            }
            bool objectIsType = isTypeReference(member->object.get());
            if (!method->isStatic && objectIsType) {
                throw BlochError(
                    ErrorCategory::Semantic, node.line, node.column,
                    "instance method '" + member->member + "' requires an object instance");
            }
            if (auto superObj = dynamic_cast<SuperExpression*>(member->object.get())) {
                (void)superObj;
                if (m_inStaticContext) {
                    throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                     "'super' cannot be used in static context");
                }
                const ClassInfo* cur = findClass(m_currentClass);
                if (!cur || cur->base.empty()) {
                    throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                     "'super' used without a base class");
                }
                if (method->isStatic) {
                    throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                     "static methods should be accessed via the type, not super");
                }
            }
            auto params = method->paramTypes;
            if (cls)
                params = substituteMany(params, cls->typeParams, searchType.typeArgs);
            checkArgs(params, member->member, node.line, node.column);
        } else if (auto superCtor = dynamic_cast<SuperExpression*>(node.callee.get())) {
            (void)superCtor;
            if (!m_inConstructor || !m_allowSuperConstructorCall) {
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "'super(...)' is only allowed as the first statement of a "
                                 "constructor");
            }
            const ClassInfo* cur = findClass(m_currentClass);
            if (!cur || cur->base.empty()) {
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "'super' used without a base class");
            }
            const ClassInfo* base = findClass(cur->base);
            bool matched = false;
            bool ambiguous = false;
            int bestCost = std::numeric_limits<int>::max();
            if (base) {
                for (const auto& ctor : base->constructors) {
                    if (!isAccessible(ctor.visibility, base->name, m_currentClass))
                        continue;
                    auto cost = paramsConversionCost(ctor.paramTypes, actualTypes);
                    if (!cost)
                        continue;
                    if (*cost < bestCost) {
                        bestCost = *cost;
                        matched = true;
                        ambiguous = false;
                    } else if (*cost == bestCost) {
                        ambiguous = true;
                    }
                }
            }
            if (!matched) {
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "no accessible base constructor matches 'super(...)'");
            }
            if (ambiguous) {
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "ambiguous base constructor call in 'super(...)'");
            }
        } else if (node.callee) {
            node.callee->accept(*this);
        }
        for (auto& arg : node.arguments) arg->accept(*this);
    }

    void SemanticAnalyser::visit(MemberAccessExpression& node) {
        if (node.object)
            node.object->accept(*this);
        auto objType = inferTypeInfo(node.object.get());
        if (objType.className.empty()) {
            throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                             "member access requires a class reference");
        }
        TypeInfo searchType = objType;
        if (objType.isTypeParam) {
            auto bound = getTypeParamBound(objType.className);
            if (!bound || bound->className.empty()) {
                throw BlochError(
                    ErrorCategory::Semantic, node.line, node.column,
                    "type parameter '" + objType.className + "' is not bound to a class type");
            }
            searchType = *bound;
        }
        const ClassInfo* cls = findClass(searchType.className);
        if (!cls) {
            throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                             "class '" + objType.className + "' not found");
        }
        bool objectIsType = isTypeReference(node.object.get());
        auto* field = findFieldInHierarchy(searchType, node.member);
        auto* method = findMethodInHierarchy(searchType, node.member);
        if (!field && !method) {
            throw BlochError(
                ErrorCategory::Semantic, node.line, node.column,
                "member '" + node.member + "' not found on class '" + objType.className + "'");
        }
        if (field) {
            if (!isAccessible(field->visibility, field->owner, m_currentClass)) {
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "member '" + node.member + "' is not accessible here");
            }
            if (!field->isStatic && objectIsType) {
                throw BlochError(
                    ErrorCategory::Semantic, node.line, node.column,
                    "instance field '" + node.member + "' cannot be accessed on a type");
            }
        } else if (method) {
            if (!isAccessible(method->visibility, method->owner, m_currentClass)) {
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "member '" + node.member + "' is not accessible here");
            }
            if (!method->isStatic && objectIsType) {
                throw BlochError(
                    ErrorCategory::Semantic, node.line, node.column,
                    "instance method '" + node.member + "' requires an object instance");
            }
            if (m_inStaticContext && method->owner == m_currentClass && !method->isStatic &&
                isThisReference(node.object.get())) {
                throw BlochError(
                    ErrorCategory::Semantic, node.line, node.column,
                    "cannot call instance method '" + node.member + "' from static context");
            }
        }
    }

    void SemanticAnalyser::visit(NewExpression& node) {
        auto cls = typeFromAst(node.classType.get());
        if (cls.isTypeParam) {
            throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                             "cannot instantiate type parameter '" + cls.className + "'");
        }
        if (!cls.className.empty()) {
            const ClassInfo* info = findClass(cls.className);
            if (!info)
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "class '" + cls.className + "' not found");
            if (info->isStatic || info->isAbstract) {
                throw BlochError(
                    ErrorCategory::Semantic, node.line, node.column,
                    "cannot instantiate static or abstract class '" + cls.className + "'");
            }
            std::vector<TypeInfo> actualTypes;
            actualTypes.reserve(node.arguments.size());
            for (auto& arg : node.arguments) actualTypes.push_back(inferTypeInfo(arg.get()));
            bool matched = false;
            bool ambiguous = false;
            int bestCost = std::numeric_limits<int>::max();
            for (const auto& ctor : info->constructors) {
                if (!isAccessible(ctor.visibility, info->name, m_currentClass))
                    continue;
                auto params = ctor.paramTypes;
                params = substituteMany(params, info->typeParams, cls.typeArgs);
                auto cost = paramsConversionCost(params, actualTypes);
                if (!cost)
                    continue;
                if (*cost < bestCost) {
                    bestCost = *cost;
                    matched = true;
                    ambiguous = false;
                } else if (*cost == bestCost) {
                    ambiguous = true;
                }
            }
            if (!matched) {
                throw BlochError(
                    ErrorCategory::Semantic, node.line, node.column,
                    "no accessible constructor found for class '" + cls.className + "'");
            }
            if (ambiguous) {
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "ambiguous constructor call for class '" + cls.className + "'");
            }
        }
        for (auto& arg : node.arguments)
            if (arg)
                arg->accept(*this);
    }

    void SemanticAnalyser::visit(ThisExpression& node) {
        if (m_currentClass.empty()) {
            throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                             "'this' may only be used inside a class instance context");
        }
        if (m_inStaticContext) {
            throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                             "'this' may not be used in static context");
        }
    }

    void SemanticAnalyser::visit(SuperExpression& node) {
        if (m_currentClass.empty()) {
            throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                             "'super' may only be used inside a class");
        }
        if (m_inStaticContext) {
            throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                             "'super' may not be used in static context");
        }
        const ClassInfo* cur = findClass(m_currentClass);
        if (!cur || cur->base.empty()) {
            throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                             "'super' used without a base class");
        }
    }

    void SemanticAnalyser::visit(IndexExpression& node) {
        if (node.collection)
            node.collection->accept(*this);
        if (node.index)
            node.index->accept(*this);
    }

    void SemanticAnalyser::visit(ArrayLiteralExpression& node) {
        for (auto& el : node.elements)
            if (el)
                el->accept(*this);
    }

    void SemanticAnalyser::visit(ParenthesizedExpression& node) {
        if (node.expression)
            node.expression->accept(*this);
    }

    void SemanticAnalyser::visit(MeasureExpression& node) {
        if (node.qubit)
            node.qubit->accept(*this);
        auto tinfo = inferTypeInfo(node.qubit.get());
        if (tinfo.value != ValueType::Unknown && tinfo.value != ValueType::Qubit) {
            throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                             "measure target must be a 'qubit'");
        }
    }

    void SemanticAnalyser::visit(AssignmentExpression& node) {
        if (isDeclared(node.name)) {
            if (isFinal(node.name)) {
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "Cannot assign to final variable '" + node.name + "'");
            }
            if (node.value) {
                TypeInfo targetType = getVariableType(node.name);
                inferDiamondTypeArguments(node.value.get(), targetType, node.line, node.column);
                auto valType = inferTypeInfo(node.value.get());
                if (valType.value == ValueType::Null) {
                    bool targetIsArray =
                        targetType.className.size() >= 2 &&
                        targetType.className.rfind("[]") == targetType.className.size() - 2;
                    bool targetIsClass = !targetType.className.empty() && !targetIsArray;
                    if (!targetIsClass) {
                        throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                         "cannot assign null to '" + node.name + "'");
                    }
                } else if (valType.value != ValueType::Unknown &&
                           !isAssignableType(targetType, valType)) {
                    throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                     "assignment to '" + node.name + "' expects '" +
                                         typeLabel(targetType) + "'");
                }
                node.value->accept(*this);
            }
            return;
        }
        if (auto field = resolveField(node.name, node.line, node.column)) {
            recordFinalFieldAssignment(*field, node.name, node.line, node.column);
            if (node.value) {
                TypeInfo targetType = field->type;
                inferDiamondTypeArguments(node.value.get(), targetType, node.line, node.column);
                auto valType = inferTypeInfo(node.value.get());
                bool fieldIsArray =
                    targetType.className.size() >= 2 &&
                    targetType.className.rfind("[]") == targetType.className.size() - 2;
                if (valType.value == ValueType::Null) {
                    if (fieldIsArray || targetType.className.empty()) {
                        throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                         "cannot assign null to field '" + node.name + "'");
                    }
                }
                if (!targetType.className.empty() && valType.value != ValueType::Null &&
                    valType.value != ValueType::Unknown && !isAssignableType(targetType, valType)) {
                    throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                     "assignment to field '" + node.name + "' expects '" +
                                         typeLabel(targetType) + "'");
                } else if (field->type.value != ValueType::Unknown &&
                           valType.value != ValueType::Unknown &&
                           !matchesPrimitive(targetType.value, valType.value)) {
                    throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                     "assignment to field '" + node.name + "' expects '" +
                                         typeToString(targetType.value) + "'");
                }
                node.value->accept(*this);
            }
            return;
        }
        throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                         "Variable '" + node.name + "' not declared");
    }

    void SemanticAnalyser::visit(MemberAssignmentExpression& node) {
        if (node.object)
            node.object->accept(*this);
        auto objType = inferTypeInfo(node.object.get());
        if (objType.className.empty()) {
            throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                             "member assignment requires a class reference");
        }
        TypeInfo searchType = objType;
        if (objType.isTypeParam) {
            auto bound = getTypeParamBound(objType.className);
            if (!bound || bound->className.empty()) {
                throw BlochError(
                    ErrorCategory::Semantic, node.line, node.column,
                    "type parameter '" + objType.className + "' is not bound to a class type");
            }
            searchType = *bound;
        }
        const ClassInfo* cls = findClass(searchType.className);
        if (!cls) {
            throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                             "class '" + objType.className + "' not found");
        }
        FieldInfo* field = findFieldInHierarchy(searchType, node.member);
        if (!field) {
            throw BlochError(
                ErrorCategory::Semantic, node.line, node.column,
                "field '" + node.member + "' not found in class '" + objType.className + "'");
        }
        if (!isAccessible(field->visibility, field->owner, m_currentClass)) {
            throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                             "field '" + node.member + "' is not accessible here");
        }
        bool objectIsType = isTypeReference(node.object.get());
        if (!field->isStatic && objectIsType) {
            throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                             "instance field '" + node.member + "' cannot be assigned via type");
        }
        if (field->isFinal) {
            bool allowed = m_inConstructor && isThisReference(node.object.get());
            if (!allowed) {
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "cannot assign to final field '" + node.member + "'");
            }
            recordFinalFieldAssignment(*field, node.member, node.line, node.column);
        }
        if (node.value) {
            TypeInfo targetType = field->type;
            if (!searchType.typeArgs.empty() && cls)
                targetType = substituteTypeParams(targetType, cls->typeParams, searchType.typeArgs);
            inferDiamondTypeArguments(node.value.get(), targetType, node.line, node.column);
            auto valType = inferTypeInfo(node.value.get());
            bool fieldIsArray = targetType.className.size() >= 2 &&
                                targetType.className.rfind("[]") == targetType.className.size() - 2;
            if (valType.value == ValueType::Null) {
                if (fieldIsArray || targetType.className.empty()) {
                    throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                     "cannot assign null to field '" + node.member + "'");
                }
            }
            if (!targetType.className.empty() && valType.value != ValueType::Null &&
                valType.value != ValueType::Unknown && !isAssignableType(targetType, valType)) {
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "assignment to field '" + node.member + "' expects '" +
                                     typeLabel(targetType) + "'");
            } else if (targetType.value != ValueType::Unknown &&
                       valType.value != ValueType::Unknown &&
                       !matchesPrimitive(targetType.value, valType.value)) {
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "assignment to field '" + node.member + "' expects '" +
                                     typeToString(targetType.value) + "'");
            }
            node.value->accept(*this);
        }
    }

    void SemanticAnalyser::visit(ArrayAssignmentExpression& node) {
        if (node.collection)
            node.collection->accept(*this);
        if (node.index)
            node.index->accept(*this);
        if (node.value)
            node.value->accept(*this);

        // Type check: array element assignment must match element type.
        auto isArrayName = [](const std::string& name) {
            return name.size() >= 2 && name.rfind("[]") == name.size() - 2;
        };

        TypeInfo collectionType = inferTypeInfo(node.collection.get());
        if (!isArrayName(collectionType.className) || collectionType.typeArgs.empty()) {
            throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                             "assignment target is not an array");
        }

        TypeInfo indexType = inferTypeInfo(node.index.get());
        if (indexType.value != ValueType::Int && indexType.value != ValueType::Long) {
            throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                             "array index must be of type 'int' or 'long'");
        }

        TypeInfo elemType = collectionType.typeArgs.front();
        TypeInfo valType = inferTypeInfo(node.value.get());

        if (valType.value == ValueType::Null) {
            // Only class types may take null.
            bool elemIsArray = isArrayName(elemType.className);
            bool elemIsClass = !elemType.className.empty() && !elemIsArray;
            if (!elemIsClass) {
                throw BlochError(
                    ErrorCategory::Semantic, node.line, node.column,
                    "cannot assign null to array element of type '" + typeLabel(elemType) + "'");
            }
            return;
        }

        auto typesCompatible =
            isAssignableType(elemType, valType) ||
            matchesPrimitive(elemType.value, valType.value) ||
            (elemType.value == ValueType::Int && valType.value == ValueType::Bit);

        if (!typesCompatible) {
            throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                             "assignment to array element expects '" + typeLabel(elemType) + "'");
        }
    }

    void SemanticAnalyser::visit(PrimitiveType&) {}
    void SemanticAnalyser::visit(NamedType&) {}
    void SemanticAnalyser::visit(ArrayType&) {}
    void SemanticAnalyser::visit(VoidType&) {}

    void SemanticAnalyser::visit(Parameter& node) {
        if (node.type)
            node.type->accept(*this);
    }

    void SemanticAnalyser::visit(TypeParameter&) {}

    void SemanticAnalyser::visit(AnnotationNode&) {}
    void SemanticAnalyser::visit(PackageDeclaration&) {}
    void SemanticAnalyser::visit(ImportDeclaration&) {}
    void SemanticAnalyser::visit(FieldDeclaration& node) {
        for (const auto& ann : node.annotations) {
            if (!ann)
                continue;
            if (ann->name == "quantum") {
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "'@quantum' may annotate functions only");
            }
            if (ann->name == "shots") {
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "'@shots(N)' can only decorate the main() function.");
            }
        }
        TypeInfo tinfo = typeFromAst(node.fieldType.get());
        if (tinfo.value == ValueType::Void) {
            throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                             "fields cannot have type 'void'");
        }

        bool savedStatic = m_inStaticContext;
        m_inStaticContext = node.isStatic;
        if (node.isFinal && node.isStatic && !node.initializer) {
            throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                             "final static field '" + node.name + "' must be initialised");
        }
        if (node.initializer) {
            validateTypedInitializer(node.name, node.fieldType.get(), node.initializer.get(),
                                     node.line, node.column);
        }
        m_inStaticContext = savedStatic;
    }

    void SemanticAnalyser::visit(MethodDeclaration& node) {
        // Only analyse bodies for now.
        TypeInfo ret = typeFromAst(node.returnType.get());
        for (const auto& ann : node.annotations) {
            if (ann && ann->name == "shots") {
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "'@shots(N)' can only decorate the main() function.");
            }
        }
        if (node.hasQuantumAnnotation) {
            bool valid = false;
            if (ret.className.empty() &&
                (ret.value == ValueType::Bit || ret.value == ValueType::Void))
                valid = true;
            if (ret.className == "bit[]")
                valid = true;
            if (!valid) {
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "'@quantum' methods must return 'bit', 'bit[]', or 'void'");
            }
        }
        auto savedReturn = m_currentReturn;
        auto savedClass = m_currentClass;
        bool savedStatic = m_inStaticContext;
        auto savedMethod = m_currentMethod;
        bool savedOverride = m_currentMethodIsOverride;
        bool savedCtor = m_inConstructor;
        bool savedDtor = m_inDestructor;
        m_currentClass = savedClass.empty() ? "" : savedClass;
        m_inStaticContext = node.isStatic;
        m_currentMethod = node.name;
        m_currentMethodIsOverride = node.isOverride;
        m_inConstructor = false;
        m_inDestructor = false;
        m_currentReturn = ret;
        m_foundReturn = false;

        beginScope();
        // 'this' binding
        if (!node.isStatic && !savedClass.empty()) {
            declare("this", true, combine(ValueType::Unknown, savedClass));
        }
        for (auto& p : node.params) {
            TypeInfo pt = typeFromAst(p->type.get());
            if (isDeclared(p->name)) {
                throw BlochError(ErrorCategory::Semantic, p->line, p->column,
                                 "'" + p->name + "' is already declared in this scope");
            }
            declare(p->name, false, pt);
            p->accept(*this);
        }
        if (node.body)
            node.body->accept(*this);
        endScope();

        m_currentReturn = savedReturn;
        m_currentClass = savedClass;
        m_inStaticContext = savedStatic;
        m_currentMethod = savedMethod;
        m_currentMethodIsOverride = savedOverride;
        m_inConstructor = savedCtor;
        m_inDestructor = savedDtor;
    }

    void SemanticAnalyser::visit(ConstructorDeclaration& node) {
        auto savedClass = m_currentClass;
        bool savedStatic = m_inStaticContext;
        bool savedCtor = m_inConstructor;
        bool savedDtor = m_inDestructor;
        auto savedMethod = m_currentMethod;
        bool savedOverride = m_currentMethodIsOverride;
        auto savedReturn = m_currentReturn;
        bool savedFoundReturn = m_foundReturn;
        bool savedAllowSuper = m_allowSuperConstructorCall;
        auto savedCtorFinalAssignments = m_constructorFinalAssignments;
        int savedCtorAssignmentDepth = m_constructorFinalAssignmentDepth;
        m_constructorFinalAssignments.clear();
        m_constructorFinalAssignmentDepth = 0;
        m_inStaticContext = false;
        m_inConstructor = true;
        m_inDestructor = false;
        m_currentMethod = "<constructor>";
        m_currentMethodIsOverride = false;
        m_currentReturn = combine(ValueType::Unknown, savedClass);
        m_foundReturn = false;
        beginScope();
        if (!savedClass.empty())
            declare("this", true, combine(ValueType::Unknown, savedClass));
        for (auto& p : node.params) {
            TypeInfo pt = typeFromAst(p->type.get());
            if (isDeclared(p->name)) {
                throw BlochError(ErrorCategory::Semantic, p->line, p->column,
                                 "'" + p->name + "' is already declared in this scope");
            }
            declare(p->name, false, pt);
            p->accept(*this);
        }
        const ClassInfo* ctorClassInfo = findClass(savedClass);
        if (ctorClassInfo && node.isDefault) {
            for (auto& p : node.params) {
                auto it = ctorClassInfo->fields.find(p->name);
                if (it == ctorClassInfo->fields.end())
                    continue;
                const FieldInfo& f = it->second;
                if (f.isStatic || !f.isFinal)
                    continue;
                if (f.hasInitializer) {
                    throw BlochError(ErrorCategory::Semantic, p->line, p->column,
                                     "default constructor cannot bind final field '" + p->name +
                                         "' because it already has a declaration initialiser");
                }
                std::string key = f.owner + "::" + p->name;
                int& assignmentCount = m_constructorFinalAssignments[key];
                assignmentCount += 1;
                if (assignmentCount > 1) {
                    throw BlochError(
                        ErrorCategory::Semantic, p->line, p->column,
                        "final field '" + p->name + "' may only be assigned once in a constructor");
                }
            }
        }
        bool superSeen = false;
        if (node.body) {
            auto& stmts = node.body->statements;
            for (size_t i = 0; i < stmts.size(); ++i) {
                bool isSuper = isSuperConstructorCall(stmts[i].get());
                if (isSuper) {
                    if (superSeen) {
                        throw BlochError(ErrorCategory::Semantic, stmts[i]->line, stmts[i]->column,
                                         "constructor may only call 'super(...)' once");
                    }
                    if (i != 0) {
                        throw BlochError(ErrorCategory::Semantic, stmts[i]->line, stmts[i]->column,
                                         "'super(...)' must be the first statement in a "
                                         "constructor");
                    }
                    superSeen = true;
                    m_allowSuperConstructorCall = true;
                    stmts[i]->accept(*this);
                    m_allowSuperConstructorCall = false;
                } else {
                    stmts[i]->accept(*this);
                    if (dynamic_cast<ReturnStatement*>(stmts[i].get()))
                        break;
                }
            }
        }
        if (ctorClassInfo && !superSeen && !ctorClassInfo->base.empty()) {
            const ClassInfo* base = findClass(ctorClassInfo->base);
            bool matched = false;
            bool ambiguous = false;
            int bestCost = std::numeric_limits<int>::max();
            const std::vector<TypeInfo> noArgs;
            if (base) {
                for (const auto& ctor : base->constructors) {
                    if (!isAccessible(ctor.visibility, base->name, m_currentClass))
                        continue;
                    auto cost = paramsConversionCost(ctor.paramTypes, noArgs);
                    if (!cost)
                        continue;
                    if (*cost < bestCost) {
                        bestCost = *cost;
                        matched = true;
                        ambiguous = false;
                    } else if (*cost == bestCost) {
                        ambiguous = true;
                    }
                }
            }
            if (!matched) {
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "no accessible base constructor matches implicit super()");
            }
            if (ambiguous) {
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "ambiguous implicit super() constructor call");
            }
        }
        if (ctorClassInfo) {
            for (const auto& entry : ctorClassInfo->fields) {
                const std::string& fieldName = entry.first;
                const FieldInfo& field = entry.second;
                if (field.isStatic || !field.isFinal || field.hasInitializer)
                    continue;
                std::string key = field.owner + "::" + fieldName;
                auto it = m_constructorFinalAssignments.find(key);
                if (it == m_constructorFinalAssignments.end() || it->second == 0) {
                    throw BlochError(
                        ErrorCategory::Semantic, node.line, node.column,
                        "final field '" + fieldName + "' must be initialised in every constructor");
                }
            }
        }
        endScope();
        m_inStaticContext = savedStatic;
        m_currentClass = savedClass;
        m_inConstructor = savedCtor;
        m_inDestructor = savedDtor;
        m_currentMethod = savedMethod;
        m_currentMethodIsOverride = savedOverride;
        m_currentReturn = savedReturn;
        m_foundReturn = savedFoundReturn;
        m_allowSuperConstructorCall = savedAllowSuper;
        m_constructorFinalAssignments = std::move(savedCtorFinalAssignments);
        m_constructorFinalAssignmentDepth = savedCtorAssignmentDepth;
    }

    void SemanticAnalyser::visit(DestructorDeclaration& node) {
        auto savedClass = m_currentClass;
        bool savedStatic = m_inStaticContext;
        bool savedCtor = m_inConstructor;
        bool savedDtor = m_inDestructor;
        auto savedMethod = m_currentMethod;
        bool savedOverride = m_currentMethodIsOverride;
        m_inStaticContext = false;
        m_inConstructor = false;
        m_inDestructor = true;
        m_currentMethod = "<destructor>";
        m_currentMethodIsOverride = false;
        beginScope();
        if (!savedClass.empty())
            declare("this", true, combine(ValueType::Unknown, savedClass));
        if (node.body)
            node.body->accept(*this);
        endScope();
        m_inStaticContext = savedStatic;
        m_currentClass = savedClass;
        m_inConstructor = savedCtor;
        m_inDestructor = savedDtor;
        m_currentMethod = savedMethod;
        m_currentMethodIsOverride = savedOverride;
    }

    void SemanticAnalyser::visit(ClassDeclaration& node) {
        // Visit member bodies minimally.
        auto savedClass = m_currentClass;
        auto savedParams = m_currentTypeParams;
        m_currentClass = node.name;
        auto it = m_classes.find(node.name);
        if (it != m_classes.end())
            m_currentTypeParams = it->second.typeParams;
        for (auto& member : node.members) {
            if (auto field = dynamic_cast<FieldDeclaration*>(member.get()))
                field->accept(*this);
            else if (auto m = dynamic_cast<MethodDeclaration*>(member.get()))
                m->accept(*this);
            else if (auto ctor = dynamic_cast<ConstructorDeclaration*>(member.get()))
                ctor->accept(*this);
            else if (auto dtor = dynamic_cast<DestructorDeclaration*>(member.get()))
                dtor->accept(*this);
        }
        m_currentClass = savedClass;
        m_currentTypeParams = savedParams;
    }

    void SemanticAnalyser::visit(FunctionDeclaration& node) {
        if (node.hasQuantumAnnotation) {
            bool returnTypeIsValid = false;
            if (auto prim = dynamic_cast<PrimitiveType*>(node.returnType.get())) {
                if (prim->name == "bit")
                    returnTypeIsValid = true;
            } else if (auto arr = dynamic_cast<ArrayType*>(node.returnType.get())) {
                if (auto elem = dynamic_cast<PrimitiveType*>(arr->elementType.get())) {
                    if (elem->name == "bit")
                        returnTypeIsValid = true;
                }
            } else if (dynamic_cast<VoidType*>(node.returnType.get())) {
                returnTypeIsValid = true;
            }

            if (!returnTypeIsValid) {
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "'@quantum' functions must return 'bit', 'bit[]', or 'void'.");
            }

            // @quantum is not allowed on the main() entry point function
            if (node.name == "main") {
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "'@quantum' cannot decorate the main() function.");
            }
        }

        int shotsCount = 0;
        for (const auto& ann : node.annotations) {
            if (ann && ann->name == "shots")
                shotsCount++;
        }
        if (shotsCount > 1) {
            throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                             "multiple '@shots' annotations are not allowed.");
        }

        if (shotsCount > 0 || node.hasShotsAnnotation) {
            bool isOnMainFunction = false;
            if (node.name == "main") {
                isOnMainFunction = true;
            }

            if (!isOnMainFunction) {
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "'@shots(N)' can only decorate the main() function.");
            }
        }

        TypeInfo ret = typeFromAst(node.returnType.get());
        m_currentReturn = ret;
        bool prevFound = m_foundReturn;
        m_foundReturn = false;

        FunctionInfo info;
        info.returnType = ret;
        for (auto& p : node.params) {
            info.paramTypes.push_back(typeFromAst(p->type.get()));
        }
        m_functionInfo[node.name] = info;

        beginScope();
        for (auto& param : node.params) {
            if (isDeclared(param->name)) {
                throw BlochError(ErrorCategory::Semantic, param->line, param->column,
                                 "'" + param->name + "' is already declared in this scope");
            }
            TypeInfo pt = typeFromAst(param->type.get());
            if (pt.value == ValueType::Void) {
                throw BlochError(ErrorCategory::Semantic, param->line, param->column,
                                 "parameters cannot have type 'void'");
            }
            declare(param->name, false, pt);
            param->accept(*this);
        }
        if (node.body)
            node.body->accept(*this);
        if (ret.value != ValueType::Void || !ret.className.empty()) {
            if (!m_foundReturn) {
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "Non-void function must have a 'return' statement.");
            }
        }
        endScope();

        m_foundReturn = prevFound;
    }

    void SemanticAnalyser::visit(Program& node) {
        // handled in analyse
    }

}  // namespace bloch::compiler
