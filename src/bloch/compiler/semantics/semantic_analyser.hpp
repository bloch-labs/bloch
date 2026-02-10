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

#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "bloch/compiler/ast/ast.hpp"
#include "bloch/compiler/semantics/type_system.hpp"
#include "bloch/support/error/bloch_error.hpp"

namespace bloch::compiler {

    // The semantic pass checks name usage, types at a coarse level,
    // function contracts, and a few language rules (like @tracked constraints).
    // It also records function signatures for later runtime validation.
    class SemanticAnalyser : public ASTVisitor {
       public:
        void analyse(Program& program);

        struct TypeInfo {
            ValueType value = ValueType::Unknown;
            std::string className;  // non-empty when referring to a user-defined class
            std::vector<TypeInfo> typeArgs;
            bool isTypeParam = false;
            bool isClass() const { return !className.empty(); }
        };

        static bool typeEquals(const TypeInfo& a, const TypeInfo& b);
        static std::string typeLabel(const TypeInfo& t);

        // Visitors
        void visit(VariableDeclaration& node) override;
        void visit(BlockStatement& node) override;
        void visit(ExpressionStatement& node) override;
        void visit(ReturnStatement& node) override;
        void visit(IfStatement& node) override;
        void visit(TernaryStatement& node) override;
        void visit(ForStatement& node) override;
        void visit(WhileStatement& node) override;
        void visit(EchoStatement& node) override;
        void visit(ResetStatement& node) override;
        void visit(MeasureStatement& node) override;
        void visit(DestroyStatement& node) override;
        void visit(AssignmentStatement& node) override;

        void visit(BinaryExpression& node) override;
        void visit(UnaryExpression& node) override;
        void visit(CastExpression& node) override;
        void visit(PostfixExpression& node) override;
        void visit(LiteralExpression& node) override;
        void visit(NullLiteralExpression& node) override;
        void visit(VariableExpression& node) override;
        void visit(CallExpression& node) override;
        void visit(MemberAccessExpression& node) override;
        void visit(NewExpression& node) override;
        void visit(ThisExpression& node) override;
        void visit(SuperExpression& node) override;
        void visit(IndexExpression& node) override;
        void visit(ArrayLiteralExpression& node) override;
        void visit(ParenthesizedExpression& node) override;
        void visit(MeasureExpression& node) override;
        void visit(AssignmentExpression& node) override;
        void visit(MemberAssignmentExpression& node) override;
        void visit(ArrayAssignmentExpression& node) override;

        void visit(PrimitiveType& node) override;
        void visit(NamedType& node) override;
        void visit(ArrayType& node) override;
        void visit(VoidType& node) override;

        void visit(Parameter& node) override;
        void visit(TypeParameter& node) override;
        void visit(AnnotationNode& node) override;
        void visit(PackageDeclaration& node) override;
        void visit(ImportDeclaration& node) override;
        void visit(FieldDeclaration& node) override;
        void visit(MethodDeclaration& node) override;
        void visit(ConstructorDeclaration& node) override;
        void visit(DestructorDeclaration& node) override;
        void visit(ClassDeclaration& node) override;
        void visit(FunctionDeclaration& node) override;
        void visit(Program& node) override;

       private:
        SymbolTable m_symbols;
        TypeInfo m_currentReturn;
        bool m_foundReturn = false;
        std::unordered_set<std::string> m_functions;

        struct FunctionInfo {
            TypeInfo returnType;
            std::vector<TypeInfo> paramTypes;
        };
        std::unordered_map<std::string, FunctionInfo> m_functionInfo;

        struct FieldInfo {
            Visibility visibility = Visibility::Public;
            bool isStatic = false;
            bool isFinal = false;
            bool hasInitializer = false;
            bool isTracked = false;
            TypeInfo type;
            std::string owner;
            int line = 0;
            int column = 0;
        };

        struct MethodInfo {
            Visibility visibility = Visibility::Public;
            bool isStatic = false;
            bool isVirtual = false;
            bool isOverride = false;
            bool hasBody = false;
            bool isDefault = false;  // used for constructors
            std::string name;
            std::string signature;
            TypeInfo returnType;
            std::vector<TypeInfo> paramTypes;
            std::string owner;
            int line = 0;
            int column = 0;
        };

        struct ClassInfo {
            std::string name;
            std::string base;
            bool isStatic = false;
            bool isAbstract = false;
            bool hasDestructor = true;       // implicit default exists
            bool hasUserDestructor = false;  // true if explicitly declared
            struct TypeParamInfo {
                std::string name;
                TypeInfo bound;
            };
            std::vector<TypeParamInfo> typeParams;
            std::vector<std::string> abstractMethods;
            std::unordered_map<std::string, FieldInfo> fields;
            std::unordered_map<std::string, std::vector<MethodInfo>> methods;
            std::unordered_set<std::string> methodSignatures;
            std::vector<MethodInfo> constructors;
            std::vector<ConstructorDeclaration*> ctorDecls;  // raw pointers owned by AST
            int line = 0;
            int column = 0;
        };

        std::unordered_map<std::string, ClassInfo> m_classes;
        std::string m_currentClass;
        bool m_inStaticContext = false;
        bool m_inConstructor = false;
        bool m_inDestructor = false;
        bool m_allowSuperConstructorCall = false;
        std::string m_currentMethod;
        bool m_currentMethodIsOverride = false;

        // Track full type information (including generic arguments) alongside the symbol table.
        std::vector<std::unordered_map<std::string, TypeInfo>> m_typeStack;

        // Helpers
        void beginScope();
        void endScope();
        void declare(const std::string& name, bool isFinal, const TypeInfo& type,
                     bool isTypeName = false);
        bool isDeclared(const std::string& name) const;
        void declareFunction(const std::string& name);
        bool isFunctionDeclared(const std::string& name) const;
        bool isFinal(const std::string& name) const;
        size_t getFunctionParamCount(const std::string& name) const;
        std::vector<TypeInfo> getFunctionParamTypes(const std::string& name) const;
        TypeInfo getVariableType(const std::string& name) const;
        std::string getVariableClassName(const std::string& name) const;
        bool returnsVoid(const std::string& name) const;
        TypeInfo typeFromAst(Type* typeNode) const;
        static TypeInfo combine(ValueType prim, const std::string& cls);
        TypeInfo inferTypeInfo(Expression* expr) const;
        void buildClassRegistry(Program& program);
        const ClassInfo* findClass(const std::string& name) const;
        MethodInfo* findMethodInHierarchy(const TypeInfo& classType, const std::string& method,
                                          const std::vector<TypeInfo>* params = nullptr) const;
        FieldInfo* findFieldInHierarchy(const TypeInfo& classType, const std::string& field) const;
        const FieldInfo* resolveField(const std::string& name, int line, int column) const;
        bool isSubclassOf(const std::string& derived, const std::string& base) const;
        int inheritanceDistance(const std::string& derived, const std::string& base) const;
        bool isAssignableType(const TypeInfo& expected, const TypeInfo& actual) const;
        bool paramsAssignable(const std::vector<TypeInfo>& expected,
                              const std::vector<TypeInfo>& actual) const;
        std::optional<int> conversionCost(const TypeInfo& expected, const TypeInfo& actual) const;
        std::optional<int> paramsConversionCost(const std::vector<TypeInfo>& expected,
                                                const std::vector<TypeInfo>& actual) const;
        void validateOverrides(ClassInfo& info);
        void validateAbstractness(ClassInfo& info);
        bool isAccessible(Visibility visibility, const std::string& owner,
                          const std::string& accessor) const;
        bool isTypeReference(Expression* expr) const;
        bool isThisReference(Expression* expr) const;
        bool isSuperConstructorCall(Statement* stmt) const;
        std::unique_ptr<Type> typeFromTypeInfo(const TypeInfo& typeInfo) const;
        void inferDiamondTypeArguments(Expression* initializer, const TypeInfo& expectedType,
                                       int line, int column);
        void validateTypedInitializer(const std::string& name, Type* declaredType,
                                      Expression* initializer, int line, int column);
        void recordFinalFieldAssignment(const FieldInfo& field, const std::string& fieldName,
                                        int line, int column);

        // Type inference
        std::optional<int> evaluateConstInt(Expression* expr) const;

        // Generics helpers
        std::vector<ClassInfo::TypeParamInfo> m_currentTypeParams;
        bool m_inClassRegistryBuild = false;
        TypeInfo substituteTypeParams(const TypeInfo& t,
                                      const std::vector<ClassInfo::TypeParamInfo>& params,
                                      const std::vector<TypeInfo>& args) const;
        std::vector<TypeInfo> substituteMany(const std::vector<TypeInfo>& types,
                                             const std::vector<ClassInfo::TypeParamInfo>& params,
                                             const std::vector<TypeInfo>& args) const;
        void validateTypeApplication(const TypeInfo& t, int line, int column) const;
        std::optional<TypeInfo> getTypeParamBound(const std::string& name) const;

        // Tracks final-field assignments in the constructor currently being analysed.
        std::unordered_map<std::string, int> m_constructorFinalAssignments;
        int m_constructorFinalAssignmentDepth = 0;
    };

}  // namespace bloch::compiler
