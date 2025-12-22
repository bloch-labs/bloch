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

#pragma once

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#include "bloch/core/ast/ast.hpp"
#include "bloch/runtime/qasm_simulator.hpp"

namespace bloch::runtime {

    class RuntimeEvaluator;
    struct RuntimeClass;
    struct Object;

    using core::AnnotationNode;
    using core::ArrayAssignmentExpression;
    using core::ArrayLiteralExpression;
    using core::ArrayType;
    using core::AssignmentExpression;
    using core::AssignmentStatement;
    using core::BinaryExpression;
    using core::BlockStatement;
    using core::CallExpression;
    using core::DestroyStatement;
    using core::ConstructorDeclaration;
    using core::DestructorDeclaration;
    using core::FieldDeclaration;
    using core::EchoStatement;
    using core::Expression;
    using core::ExpressionStatement;
    using core::ForStatement;
    using core::FunctionDeclaration;
    using core::IfStatement;
    using core::IndexExpression;
    using core::LiteralExpression;
    using core::MemberAccessExpression;
    using core::MemberAssignmentExpression;
    using core::MeasureExpression;
    using core::MeasureStatement;
    using core::MethodDeclaration;
    using core::NewExpression;
    using core::Parameter;
    using core::ParenthesizedExpression;
    using core::PostfixExpression;
    using core::PrimitiveType;
    using core::Program;
    using core::NamedType;
    using core::ResetStatement;
    using core::ReturnStatement;
    using core::Statement;
    using core::SuperExpression;
    using core::ThisExpression;
    using core::TernaryStatement;
    using core::Type;
    using core::UnaryExpression;
    using core::VariableDeclaration;
    using core::VariableExpression;
    using core::VoidType;
    using core::WhileStatement;

    // Runtime values carry both a discriminant and storage. Arrays are
    // represented as std::vector<> of the appropriate primitive.
    struct Value {
        enum class Type {
            Int,
            Float,
            Bit,
            String,
            Char,
            Qubit,
            // Arrays
            IntArray,
            FloatArray,
            BitArray,
            StringArray,
            CharArray,
            QubitArray,
            Object,
            ObjectArray,
            ClassRef,
            Void
        };
        Type type = Type::Void;
        int intValue = 0;
        double floatValue = 0.0;
        int bitValue = 0;
        std::string stringValue = "";
        char charValue = '\0';
        int qubit = -1;
        std::vector<int> intArray;
        std::vector<double> floatArray;
        std::vector<int> bitArray;
        std::vector<std::string> stringArray;
        std::vector<char> charArray;
        std::vector<int> qubitArray;
        std::shared_ptr<Object> objectValue;
        std::vector<std::shared_ptr<Object>> objectArray;
        RuntimeClass* classRef = nullptr;
        std::string className;

        Value() = default;
        explicit Value(Type t) : type(t) {}
        Value(Type t, int intVal, double floatVal = 0.0, int bitVal = 0, std::string strVal = "",
              char charVal = '\0')
            : type(t),
              intValue(intVal),
              floatValue(floatVal),
              bitValue(bitVal),
              stringValue(std::move(strVal)),
              charValue(charVal) {}
    };

    struct RuntimeTypeInfo {
        Value::Type kind = Value::Type::Void;
        std::string className;
    };

    struct RuntimeField {
        std::string name;
        RuntimeTypeInfo type;
        bool isStatic = false;
        bool isFinal = false;
        bool isTracked = false;
        Expression* initializer = nullptr;
        bool hasInitializer = false;
        int arraySize = -1;
        int line = 0;
        int column = 0;
        size_t offset = 0;
    };

    struct RuntimeMethod;
    struct RuntimeConstructor {
        ConstructorDeclaration* decl = nullptr;
        std::vector<RuntimeTypeInfo> params;
    };

    struct RuntimeMethod {
        MethodDeclaration* decl = nullptr;
        bool isStatic = false;
        bool isVirtual = false;
        bool isOverride = false;
        size_t vtableIndex = 0;
        RuntimeClass* owner = nullptr;
    };

    struct RuntimeClass {
        std::string name;
        RuntimeClass* base = nullptr;
        bool isStatic = false;
        bool isAbstract = false;
        bool hasDestructor = false;
        bool hasTrackedFields = false;
        DestructorDeclaration* destructorDecl = nullptr;
        std::vector<RuntimeField> instanceFields;
        std::vector<RuntimeField> staticFields;
        std::vector<Value> staticStorage;
        std::unordered_map<std::string, size_t> instanceFieldIndex;
        std::unordered_map<std::string, size_t> staticFieldIndex;
        std::unordered_map<std::string, RuntimeMethod> methods;
        std::vector<RuntimeMethod*> vtable;
        std::vector<RuntimeConstructor> constructors;
    };

    struct Object {
        RuntimeClass* cls = nullptr;
        std::vector<Value> fields;
        bool skipDestructor = false;
        bool destroyed = false;
        RuntimeEvaluator* owner = nullptr;
        bool marked = false;
    };

    // Interpreter that walks the AST and simulates quantum bits via
    // QasmSimulator. It also tracks @tracked variables and defers echo output
    // until warnings have been printed.
    class RuntimeEvaluator {
       public:
        explicit RuntimeEvaluator(bool collectQasmLog = true) : m_collectQasmLog(collectQasmLog) {}
        ~RuntimeEvaluator();
        void execute(Program& program);
        const std::unordered_map<const Expression*, std::vector<int>>& measurements() const {
            return m_measurements;
        }
        std::string getQasm() const { return m_sim.getQasm(); }
        size_t heapObjectCount();

       private:
        QasmSimulator m_sim;
        bool m_collectQasmLog = true;
        std::unordered_map<std::string, FunctionDeclaration*> m_functions;
        struct VarEntry {
            Value value;
            bool tracked = false;
            bool initialized = false;
        };
        std::vector<std::unordered_map<std::string, VarEntry>> m_env;
        Value m_returnValue;
        bool m_hasReturn = false;
        std::unordered_map<const Expression*, std::vector<int>> m_measurements;
        std::unordered_map<std::string, std::unordered_map<std::string, int>> m_trackedCounts;
        bool m_echoEnabled = true;
        bool m_warnOnExit = true;
        bool m_executed = false;  // single-use guard
        // Class runtime metadata and heap tracking
        std::unordered_map<std::string, std::shared_ptr<RuntimeClass>> m_classTable;
        std::vector<std::weak_ptr<Object>> m_heap;
        RuntimeClass* m_currentClassCtx = nullptr;
        bool m_inStaticContext = false;
        bool m_inConstructor = false;
        bool m_inDestructor = false;
        std::atomic<bool> m_gcRequested{false};
        std::atomic<bool> m_stopGc{false};
        std::thread m_gcThread;
        std::condition_variable m_gcCv;
        std::mutex m_gcMutex;
        std::mutex m_heapMutex;
        size_t m_allocSinceGc = 0;
        // Buffer for echo outputs so logs (INFO/WARNING/ERROR)
        // can be displayed first before normal program output.
        std::vector<std::string> m_echoBuffer;
        struct QubitInfo {
            std::string name;
            bool measured;
        };
        std::vector<QubitInfo> m_qubits;
        // Last measured value per qubit index (-1 if never measured)
        std::vector<int> m_lastMeasurement;

        // Core interpreter operations
        Value eval(Expression* expr);
        void exec(Statement* stmt);
        Value call(FunctionDeclaration* fn, const std::vector<Value>& args);
        Value lookup(const std::string& name);
        void assign(const std::string& name, const Value& v);

        // Qubit bookkeeping
        int allocateTrackedQubit(const std::string& name);
        void markMeasured(int index);
        void unmarkMeasured(int index);
        void ensureQubitActive(int index, int line, int column);
        void ensureQubitExists(int index, int line, int column);
        void warnUnmeasured() const;

        // Class runtime helpers
        RuntimeTypeInfo typeInfoFromAst(Type* type) const;
        Value defaultValueForField(const RuntimeField& field, const std::string& ownerLabel);
        void buildClassTable(Program& program);
        RuntimeClass* findClass(const std::string& name) const;
        RuntimeMethod* findMethod(RuntimeClass* cls, const std::string& name);
        RuntimeField* findInstanceField(RuntimeClass* cls, const std::string& name);
        RuntimeField* findStaticField(RuntimeClass* cls, const std::string& name);
        void initStaticFields(RuntimeClass* cls);
        void ensureGcThread();
        void requestGc();
        void runCycleCollector();
        void markValue(const Value& v);
        void markObject(const std::shared_ptr<Object>& obj);
        void destroyObject(Object* obj, bool runUserDestructor);
        Value callMethod(RuntimeMethod* method, RuntimeClass* staticDispatchClass,
                         const std::shared_ptr<Object>& receiver, const std::vector<Value>& args);
        void runConstructorChain(RuntimeClass* cls, const std::shared_ptr<Object>& obj,
                                 ConstructorDeclaration* ctor,
                                 const std::vector<Value>& args);
        void runFieldInitialisers(RuntimeClass* cls, const std::shared_ptr<Object>& obj);
        void recordTrackedValue(const std::string& name, const Value& v);
        std::shared_ptr<Object> currentThisObject() const;

        // Scope & output helpers
        void beginScope();
        void endScope();
        void flushEchoes();

       public:
        void setEcho(bool enabled) { m_echoEnabled = enabled; }
        void setWarnOnExit(bool enabled) { m_warnOnExit = enabled; }
        const auto& trackedCounts() const { return m_trackedCounts; }
    };

}  // namespace bloch::runtime
