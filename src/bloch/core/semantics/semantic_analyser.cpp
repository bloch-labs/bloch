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

#include <algorithm>
#include <functional>
#include <iostream>

#include "bloch/core/semantics/built_ins.hpp"

namespace bloch::core {

    using support::BlochError;
    using support::ErrorCategory;

    SemanticAnalyser::TypeInfo SemanticAnalyser::combine(ValueType prim, const std::string& cls) {
        TypeInfo t;
        t.value = prim;
        t.className = cls;
        return t;
    }

    SemanticAnalyser::TypeInfo SemanticAnalyser::typeFromAst(Type* typeNode) const {
        if (!typeNode)
            return combine(ValueType::Unknown, "");
        if (auto prim = dynamic_cast<PrimitiveType*>(typeNode))
            return combine(typeFromString(prim->name), "");
        if (auto named = dynamic_cast<NamedType*>(typeNode)) {
            std::string cls = named->nameParts.empty() ? "" : named->nameParts.back();
            return combine(ValueType::Unknown, cls);
        }
        if (dynamic_cast<VoidType*>(typeNode))
            return combine(ValueType::Void, "");
        if (auto arr = dynamic_cast<ArrayType*>(typeNode)) {
            auto elem = typeFromAst(arr->elementType.get());
            if (!elem.className.empty())
                return combine(ValueType::Unknown, elem.className + "[]");
            return combine(ValueType::Unknown, typeToString(elem.value) + "[]");
        }
        return combine(ValueType::Unknown, "");
    }

    const SemanticAnalyser::ClassInfo* SemanticAnalyser::findClass(const std::string& name) const {
        auto it = m_classes.find(name);
        if (it != m_classes.end())
            return &it->second;
        return nullptr;
    }

    SemanticAnalyser::MethodInfo* SemanticAnalyser::findMethodInHierarchy(
        const std::string& className, const std::string& method) const {
        const ClassInfo* cur = findClass(className);
        while (cur) {
            auto mit = cur->methods.find(method);
            if (mit != cur->methods.end())
                return const_cast<MethodInfo*>(&mit->second);
            if (cur->base.empty())
                break;
            cur = findClass(cur->base);
        }
        return nullptr;
    }

    SemanticAnalyser::FieldInfo* SemanticAnalyser::findFieldInHierarchy(
        const std::string& className, const std::string& field) const {
        const ClassInfo* cur = findClass(className);
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
                                                                      int line,
                                                                      int column) const {
        if (m_currentClass.empty())
            return nullptr;
        FieldInfo* field = findFieldInHierarchy(m_currentClass, name);
        if (!field)
            return nullptr;
        if (!isAccessible(field->visibility, field->owner, m_currentClass)) {
            throw BlochError(ErrorCategory::Semantic, line, column,
                             "field '" + name + "' is not accessible here");
        }
        if (m_inStaticContext && !field->isStatic) {
            throw BlochError(ErrorCategory::Semantic, line, column,
                             "instance field '" + name + "' cannot be referenced in a static "
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

    void SemanticAnalyser::validateOverrides(ClassInfo& info) {
        if (info.base.empty()) {
            for (auto& kv : info.methods) {
                auto& m = kv.second;
                if (m.isOverride) {
                    throw BlochError(ErrorCategory::Semantic, m.line, m.column,
                                     "'" + kv.first + "' marked override but class has no base");
                }
                if (m.isStatic && m.isVirtual) {
                    throw BlochError(ErrorCategory::Semantic, m.line, m.column,
                                     "static method '" + kv.first + "' cannot be virtual");
                }
            }
            return;
        }
        for (auto& kv : info.methods) {
            auto& name = kv.first;
            auto& m = kv.second;
            if (m.isStatic && (m.isVirtual || m.isOverride)) {
                throw BlochError(
                    ErrorCategory::Semantic, m.line, m.column,
                    "static method '" + name + "' cannot be declared virtual or override");
            }
            const MethodInfo* baseMethod = findMethodInHierarchy(info.base, name);
            if (m.isOverride) {
                if (!baseMethod) {
                    throw BlochError(ErrorCategory::Semantic, m.line, m.column,
                                     "'" + name + "' marked override but base method not found");
                }
                if (!baseMethod->isVirtual) {
                    throw BlochError(ErrorCategory::Semantic, m.line, m.column,
                                     "'" + name + "' overrides a non-virtual base method");
                }
                if (baseMethod->isStatic) {
                    throw BlochError(ErrorCategory::Semantic, m.line, m.column,
                                     "'" + name + "' cannot override a static base method");
                }
                if (baseMethod->paramTypes.size() != m.paramTypes.size())
                    throw BlochError(ErrorCategory::Semantic, m.line, m.column,
                                     "parameter count mismatch overriding '" + name + "'");
                for (size_t i = 0; i < m.paramTypes.size(); ++i) {
                    if (baseMethod->paramTypes[i].className != m.paramTypes[i].className ||
                        baseMethod->paramTypes[i].value != m.paramTypes[i].value) {
                        throw BlochError(ErrorCategory::Semantic, m.line, m.column,
                                         "parameter mismatch overriding '" + name + "'");
                    }
                }
                if (baseMethod->returnType.className != m.returnType.className ||
                    baseMethod->returnType.value != m.returnType.value) {
                    throw BlochError(ErrorCategory::Semantic, m.line, m.column,
                                     "return type mismatch overriding '" + name + "'");
                }
            }
            if (m.isVirtual && m.isStatic) {
                throw BlochError(ErrorCategory::Semantic, m.line, m.column,
                                 "static method '" + name + "' cannot be virtual");
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
            auto& name = kv.first;
            auto& m = kv.second;
            if (m.isVirtual && !m.hasBody) {
                required.push_back(name);
            }
            if (m.hasBody) {
                auto it = std::find(required.begin(), required.end(), name);
                if (it != required.end()) {
                    const MethodInfo* baseMethod = findMethodInHierarchy(info.base, name);
                    if (baseMethod) {
                        if (m.isStatic) {
                            throw BlochError(ErrorCategory::Semantic, m.line, m.column,
                                             "static method '" + name +
                                                 "' cannot implement abstract base method");
                        }
                        if (baseMethod->paramTypes.size() != m.paramTypes.size() ||
                            baseMethod->returnType.className != m.returnType.className ||
                            baseMethod->returnType.value != m.returnType.value) {
                            throw BlochError(ErrorCategory::Semantic, m.line, m.column,
                                             "implementation of abstract method '" + name +
                                                 "' has incompatible signature");
                        }
                        for (size_t i = 0; i < m.paramTypes.size(); ++i) {
                            if (m.paramTypes[i].className != baseMethod->paramTypes[i].className ||
                                m.paramTypes[i].value != baseMethod->paramTypes[i].value) {
                                throw BlochError(ErrorCategory::Semantic, m.line, m.column,
                                                 "implementation of abstract method '" + name +
                                                     "' has incompatible signature");
                            }
                        }
                    }
                    required.erase(it);
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

    void SemanticAnalyser::buildClassRegistry(Program& program) {
        m_classes.clear();
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
            if (!clsNode->baseName.empty())
                info.base = clsNode->baseName.back();
            info.isStatic = clsNode->isStatic;
            info.isAbstract = clsNode->isAbstract;
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
                    if (info.methods.count(method->name)) {
                        throw BlochError(
                            ErrorCategory::Semantic, method->line, method->column,
                            "duplicate method '" + method->name + "' in class '" + info.name + "'");
                    }
                    if (info.isStatic && !method->isStatic) {
                        throw BlochError(
                            ErrorCategory::Semantic, method->line, method->column,
                            "static class '" + info.name + "' cannot declare instance methods");
                    }
                    MethodInfo m;
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
                    if (m.isStatic && (m.isVirtual || m.isOverride)) {
                        throw BlochError(
                            ErrorCategory::Semantic, method->line, method->column,
                            "static method '" + method->name + "' cannot be virtual or override");
                    }
                    info.methods[method->name] = m;
                    if (method->isVirtual && !m.hasBody)
                        info.abstractMethods.push_back(method->name);
                } else if (auto ctor = dynamic_cast<ConstructorDeclaration*>(member.get())) {
                    if (info.isStatic) {
                        throw BlochError(
                            ErrorCategory::Semantic, ctor->line, ctor->column,
                            "static class '" + info.name + "' cannot declare constructors");
                    }
                    MethodInfo ctorInfo;
                    ctorInfo.visibility = ctor->visibility;
                    ctorInfo.hasBody = ctor->body != nullptr;
                    ctorInfo.owner = info.name;
                    ctorInfo.line = ctor->line;
                    ctorInfo.column = ctor->column;
                    for (auto& p : ctor->params)
                        ctorInfo.paramTypes.push_back(typeFromAst(p->type.get()));
                    ctorInfo.returnType = combine(ValueType::Unknown, info.name);
                    info.constructors.push_back(ctorInfo);
                } else if (auto dtor = dynamic_cast<DestructorDeclaration*>(member.get())) {
                    if (info.isStatic) {
                        throw BlochError(
                            ErrorCategory::Semantic, dtor->line, dtor->column,
                            "static class '" + info.name + "' cannot declare destructors");
                    }
                    if (info.hasDestructor) {
                        throw BlochError(
                            ErrorCategory::Semantic, dtor->line, dtor->column,
                            "class '" + info.name + "' cannot declare multiple destructors");
                    }
                    info.hasDestructor = true;
                }
            }
            if (info.constructors.empty()) {
                for (const auto& [fname, f] : info.fields) {
                    if (f.isFinal && !f.hasInitializer) {
                        throw BlochError(ErrorCategory::Semantic, f.line, f.column,
                                         "final field '" + fname +
                                             "' must be initialised or assigned in a constructor");
                    }
                }
            }
            m_classes[info.name] = std::move(info);
        }

        for (auto& [name, info] : m_classes) {
            if (!info.base.empty() && !m_classes.count(info.base)) {
                throw BlochError(ErrorCategory::Semantic, 0, 0,
                                 "base class '" + info.base + "' not found for '" + name + "'");
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
    }

    void SemanticAnalyser::beginScope() { m_symbols.beginScope(); }
    void SemanticAnalyser::endScope() { m_symbols.endScope(); }

    void SemanticAnalyser::declare(const std::string& name, bool isFinal, const TypeInfo& type,
                                   bool isTypeName) {
        m_symbols.declare(name, isFinal, type.value, type.className, isTypeName);
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
        if (auto lit = dynamic_cast<LiteralExpression*>(expr))
            return combine(typeFromString(lit->literalType), "");
        if (auto var = dynamic_cast<VariableExpression*>(expr)) {
            TypeInfo local = combine(m_symbols.getType(var->name), m_symbols.getClassName(var->name));
            if (local.value != ValueType::Unknown || !local.className.empty())
                return local;
            if (auto field = resolveField(var->name, var->line, var->column))
                return field->type;
            return combine(ValueType::Unknown, "");
        }
        if (dynamic_cast<ThisExpression*>(expr))
            return combine(ValueType::Unknown, m_currentClass);
        if (auto par = dynamic_cast<ParenthesizedExpression*>(expr))
            return inferTypeInfo(par->expression.get());
        if (dynamic_cast<MeasureExpression*>(expr))
            return combine(ValueType::Bit, "");
        if (dynamic_cast<SuperExpression*>(expr)) {
            const ClassInfo* cur = findClass(m_currentClass);
            if (cur && !cur->base.empty())
                return combine(ValueType::Unknown, cur->base);
            return combine(ValueType::Unknown, "");
        }
        if (auto call = dynamic_cast<CallExpression*>(expr)) {
            if (auto callee = dynamic_cast<VariableExpression*>(call->callee.get())) {
                auto it = m_functionInfo.find(callee->name);
                if (it != m_functionInfo.end())
                    return it->second.returnType;
                auto bi = builtInGates.find(callee->name);
                if (bi != builtInGates.end())
                    return combine(bi->second.returnType, "");
                if (!m_currentClass.empty()) {
                    auto* method = findMethodInHierarchy(m_currentClass, callee->name);
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
                    auto* method = findMethodInHierarchy(obj.className, mem->member);
                    if (method)
                        return method->returnType;
                }
            }
            return combine(ValueType::Unknown, "");
        }
        if (auto bin = dynamic_cast<BinaryExpression*>(expr)) {
            auto lt = inferTypeInfo(bin->left.get());
            auto rt = inferTypeInfo(bin->right.get());
            if (bin->op == "==" || bin->op == "!=" || bin->op == "<" || bin->op == ">" ||
                bin->op == "<=" || bin->op == ">=" || bin->op == "&&" || bin->op == "||")
                return combine(ValueType::Bit, "");
            if (bin->op == "+" && (lt.value == ValueType::String || rt.value == ValueType::String))
                return combine(ValueType::String, "");
            if (bin->op == "+" || bin->op == "-" || bin->op == "*" || bin->op == "/" ||
                bin->op == "%") {
                if (lt.value == ValueType::Float || rt.value == ValueType::Float)
                    return combine(ValueType::Float, "");
                return combine(ValueType::Int, "");
            }
            if (bin->op == "&" || bin->op == "|" || bin->op == "^") {
                if (lt.value == ValueType::Bit && rt.value == ValueType::Bit)
                    return combine(ValueType::Bit, "");
            }
            return combine(ValueType::Unknown, "");
        }
        if (auto un = dynamic_cast<UnaryExpression*>(expr)) {
            auto rt = inferTypeInfo(un->right.get());
            if (un->op == "-")
                return combine((rt.value == ValueType::Float) ? ValueType::Float : ValueType::Int,
                               "");
            if (un->op == "!")
                return combine(ValueType::Bit, "");
            if (un->op == "~")
                return combine((rt.value == ValueType::Bit) ? ValueType::Bit : ValueType::Unknown,
                               "");
            return combine(ValueType::Unknown, "");
        }
        if (auto post = dynamic_cast<PostfixExpression*>(expr)) {
            if (auto v = dynamic_cast<VariableExpression*>(post->left.get())) {
                TypeInfo local =
                    combine(m_symbols.getType(v->name), m_symbols.getClassName(v->name));
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
                auto* field = findFieldInHierarchy(obj.className, mem->member);
                if (field)
                    return field->type;
                auto* method = findMethodInHierarchy(obj.className, mem->member);
                if (method)
                    return method->returnType;
            }
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
        if (!program.imports.empty()) {
            auto* first = program.imports.front().get();
            int line = first ? first->line : 0;
            int col = first ? first->column : 0;
            throw BlochError(ErrorCategory::Semantic, line, col,
                             "imports are parsed but not yet supported by the semantic analyser");
        }
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
            if (auto primType = tinfo.value; primType != ValueType::Unknown) {
                ValueType initT = inferTypeInfo(node.initializer.get()).value;
                if (initT != ValueType::Unknown && initT != primType) {
                    if (primType == ValueType::Bit) {
                        if (auto lit = dynamic_cast<LiteralExpression*>(node.initializer.get())) {
                            if (lit->literalType == "int") {
                                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                                 "bit literals must be 0b or 1b");
                            }
                        }
                    } else if (primType == ValueType::Float) {
                        if (auto lit = dynamic_cast<LiteralExpression*>(node.initializer.get())) {
                            if (lit->literalType == "int") {
                                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                                 "float literals must end with 'f'");
                            }
                        }
                    }
                    throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                     "initializer for '" + node.name + "' expected '" +
                                         typeToString(primType) + "' but got '" +
                                         typeToString(initT) + "'");
                }
            } else if (!tinfo.className.empty()) {
                auto initT = inferTypeInfo(node.initializer.get());
                if (!initT.className.empty() && initT.className != tinfo.className) {
                    throw BlochError(
                        ErrorCategory::Semantic, node.line, node.column,
                        "initializer for '" + node.name + "' expected '" + tinfo.className + "'");
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
            auto actual = inferTypeInfo(node.value.get());
            if (!isVoid && (actual.value != m_currentReturn.value ||
                            actual.className != m_currentReturn.className)) {
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "return type mismatch");
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
        if (tinfo.value != ValueType::Unknown && tinfo.value != ValueType::Qubit) {
            throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                             "measure target must be a 'qubit'");
        }
    }

    void SemanticAnalyser::visit(DestroyStatement& node) {
        if (node.target) {
            auto t = inferTypeInfo(node.target.get());
            if (t.className.empty()) {
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
            if (node.value)
                node.value->accept(*this);
            return;
        }
        if (auto field = resolveField(node.name, node.line, node.column)) {
            if (field->isFinal) {
                bool allowed = m_inConstructor && !field->isStatic;
                if (!allowed) {
                    throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                     "cannot assign to final field '" + node.name + "'");
                }
            }
            if (node.value) {
                auto valType = inferTypeInfo(node.value.get());
                if (!field->type.className.empty()) {
                    if (valType.className != field->type.className) {
                        throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                         "assignment to field '" + node.name + "' expects '" +
                                             field->type.className + "'");
                    }
                } else if (field->type.value != ValueType::Unknown &&
                           valType.value != ValueType::Unknown &&
                           field->type.value != valType.value) {
                    throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                     "assignment to field '" + node.name + "' expects '" +
                                         typeToString(field->type.value) + "'");
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
    }

    void SemanticAnalyser::visit(UnaryExpression& node) {
        if (node.right)
            node.right->accept(*this);
    }

    void SemanticAnalyser::visit(PostfixExpression& node) {
        if (auto var = dynamic_cast<VariableExpression*>(node.left.get())) {
            if (isDeclared(var->name)) {
                if (isFinal(var->name)) {
                    throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                     "Cannot modify final variable '" + var->name + "'");
                }
                ValueType t = m_symbols.getType(var->name);
                if (t != ValueType::Int) {
                    throw BlochError(
                        ErrorCategory::Semantic, node.line, node.column,
                        "Postfix operator '" + node.op + "' requires variable of type 'int'");
                }
                return;
            }
            if (auto field = resolveField(var->name, node.line, node.column)) {
                if (field->isFinal) {
                    throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                     "Cannot modify final field '" + var->name + "'");
                }
                if (field->type.value != ValueType::Int) {
                    throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                     "Postfix operator '" + node.op +
                                         "' requires variable of type 'int'");
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

    void SemanticAnalyser::visit(VariableExpression& node) {
        if (isDeclared(node.name) || isFunctionDeclared(node.name))
            return;
        if (resolveField(node.name, node.line, node.column))
            return;
        throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                         "Variable '" + node.name + "' not declared");
    }

    void SemanticAnalyser::visit(CallExpression& node) {
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
                auto actual = inferTypeInfo(arg.get());
                if (!expected.className.empty()) {
                    if (!actual.className.empty() && actual.className != expected.className) {
                        throw BlochError(ErrorCategory::Semantic, arg->line, arg->column,
                                         "argument #" + std::to_string(i + 1) + " to '" + name +
                                             "' expected '" + expected.className + "'");
                    }
                } else if (expected.value != ValueType::Unknown &&
                           actual.value != ValueType::Unknown && expected.value != actual.value) {
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
                    methodInfo = findMethodInHierarchy(m_currentClass, var->name);
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
            const ClassInfo* cls = findClass(objType.className);
            if (!cls) {
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "class '" + objType.className + "' not found");
            }
            MethodInfo* method = findMethodInHierarchy(objType.className, member->member);
            if (!method) {
                if (findFieldInHierarchy(objType.className, member->member)) {
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
            checkArgs(method->paramTypes, member->member, node.line, node.column);
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
            bool matched = base && base->constructors.empty() &&
                           node.arguments.empty();  // default ctor allowed
            if (base) {
                for (const auto& ctor : base->constructors) {
                    if (!isAccessible(ctor.visibility, base->name, m_currentClass))
                        continue;
                    if (ctor.paramTypes.size() != node.arguments.size())
                        continue;
                    bool sigOk = true;
                    for (size_t i = 0; i < ctor.paramTypes.size(); ++i) {
                        auto expected = ctor.paramTypes[i];
                        auto actual = inferTypeInfo(node.arguments[i].get());
                        if (!expected.className.empty()) {
                            if (actual.className != expected.className) {
                                sigOk = false;
                                break;
                            }
                        } else if (expected.value != ValueType::Unknown &&
                                   actual.value != ValueType::Unknown &&
                                   expected.value != actual.value) {
                            sigOk = false;
                            break;
                        }
                    }
                    if (sigOk) {
                        matched = true;
                        break;
                    }
                }
            }
            if (!matched) {
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "no accessible base constructor matches 'super(...)'");
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
        const ClassInfo* cls = findClass(objType.className);
        if (!cls) {
            throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                             "class '" + objType.className + "' not found");
        }
        bool objectIsType = isTypeReference(node.object.get());
        auto* field = findFieldInHierarchy(objType.className, node.member);
        auto* method = findMethodInHierarchy(objType.className, node.member);
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
            bool matched =
                info->constructors.empty() && node.arguments.empty();  // allow implicit default
            for (const auto& ctor : info->constructors) {
                if (!isAccessible(ctor.visibility, info->name, m_currentClass))
                    continue;
                if (ctor.paramTypes.size() != node.arguments.size())
                    continue;
                bool sigOk = true;
                for (size_t i = 0; i < ctor.paramTypes.size(); ++i) {
                    auto expected = ctor.paramTypes[i];
                    auto actual = inferTypeInfo(node.arguments[i].get());
                    if (!expected.className.empty()) {
                        if (actual.className != expected.className) {
                            sigOk = false;
                            break;
                        }
                    } else if (expected.value != ValueType::Unknown &&
                               actual.value != ValueType::Unknown &&
                               expected.value != actual.value) {
                        sigOk = false;
                        break;
                    }
                }
                if (sigOk) {
                    matched = true;
                    break;
                }
            }
            if (!matched) {
                throw BlochError(
                    ErrorCategory::Semantic, node.line, node.column,
                    "no accessible constructor found for class '" + cls.className + "'");
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
            if (node.value)
                node.value->accept(*this);
            return;
        }
        if (auto field = resolveField(node.name, node.line, node.column)) {
            if (field->isFinal) {
                bool allowed = m_inConstructor && !field->isStatic;
                if (!allowed) {
                    throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                     "cannot assign to final field '" + node.name + "'");
                }
            }
            if (node.value) {
                auto valType = inferTypeInfo(node.value.get());
                if (!field->type.className.empty()) {
                    if (valType.className != field->type.className) {
                        throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                         "assignment to field '" + node.name + "' expects '" +
                                             field->type.className + "'");
                    }
                } else if (field->type.value != ValueType::Unknown &&
                           valType.value != ValueType::Unknown &&
                           field->type.value != valType.value) {
                    throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                     "assignment to field '" + node.name + "' expects '" +
                                         typeToString(field->type.value) + "'");
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
        const ClassInfo* cls = findClass(objType.className);
        if (!cls) {
            throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                             "class '" + objType.className + "' not found");
        }
        FieldInfo* field = findFieldInHierarchy(objType.className, node.member);
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
        }
        if (node.value) {
            auto valType = inferTypeInfo(node.value.get());
            if (!field->type.className.empty()) {
                if (valType.className != field->type.className) {
                    throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                     "assignment to field '" + node.member + "' expects '" +
                                         field->type.className + "'");
                }
            } else if (field->type.value != ValueType::Unknown &&
                       valType.value != ValueType::Unknown && field->type.value != valType.value) {
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "assignment to field '" + node.member + "' expects '" +
                                     typeToString(field->type.value) + "'");
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
    }

    void SemanticAnalyser::visit(PrimitiveType&) {}
    void SemanticAnalyser::visit(NamedType&) {}
    void SemanticAnalyser::visit(ArrayType&) {}
    void SemanticAnalyser::visit(VoidType&) {}

    void SemanticAnalyser::visit(Parameter& node) {
        if (node.type)
            node.type->accept(*this);
    }

    void SemanticAnalyser::visit(AnnotationNode&) {}

    void SemanticAnalyser::visit(ImportDeclaration&) {}
    void SemanticAnalyser::visit(FieldDeclaration&) {}

    void SemanticAnalyser::visit(MethodDeclaration& node) {
        // Only analyse bodies for now.
        TypeInfo ret = typeFromAst(node.returnType.get());
        if (node.hasQuantumAnnotation) {
            bool valid = ret.className.empty() &&
                         (ret.value == ValueType::Bit || ret.value == ValueType::Void);
            if (!valid) {
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "'@quantum' methods must return 'bit' or 'void'");
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
        if (node.body) {
            bool superSeen = false;
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
        m_currentClass = node.name;
        for (auto& member : node.members) {
            if (auto m = dynamic_cast<MethodDeclaration*>(member.get()))
                m->accept(*this);
            else if (auto ctor = dynamic_cast<ConstructorDeclaration*>(member.get()))
                ctor->accept(*this);
            else if (auto dtor = dynamic_cast<DestructorDeclaration*>(member.get()))
                dtor->accept(*this);
        }
        m_currentClass.clear();
    }

    void SemanticAnalyser::visit(FunctionDeclaration& node) {
        if (node.hasQuantumAnnotation) {
            bool valid = false;
            if (auto prim = dynamic_cast<PrimitiveType*>(node.returnType.get())) {
                if (prim->name == "bit")
                    valid = true;
            } else if (dynamic_cast<VoidType*>(node.returnType.get())) {
                valid = true;
            }
            if (!valid) {
                throw BlochError(ErrorCategory::Semantic, node.line, node.column,
                                 "'@quantum' functions must return 'bit' or 'void'");
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

}  // namespace bloch::core
