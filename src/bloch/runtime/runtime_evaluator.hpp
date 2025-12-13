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

#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "bloch/core/ast/ast.hpp"
#include "bloch/runtime/qasm_simulator.hpp"

namespace bloch::runtime {

    using core::AnnotationNode;
    using core::ArrayAssignmentExpression;
    using core::ArrayLiteralExpression;
    using core::ArrayType;
    using core::AssignmentExpression;
    using core::AssignmentStatement;
    using core::BlockStatement;
    using core::BinaryExpression;
    using core::CallExpression;
    using core::EchoStatement;
    using core::Expression;
    using core::ExpressionStatement;
    using core::ForStatement;
    using core::FunctionDeclaration;
    using core::IfStatement;
    using core::IndexExpression;
    using core::LiteralExpression;
    using core::MeasureExpression;
    using core::MeasureStatement;
    using core::Parameter;
    using core::ParenthesizedExpression;
    using core::PostfixExpression;
    using core::PrimitiveType;
    using core::Program;
    using core::ResetStatement;
    using core::ReturnStatement;
    using core::Statement;
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

    // Interpreter that walks the AST and simulates quantum bits via
    // QasmSimulator. It also tracks @tracked variables and defers echo output
    // until warnings have been printed.
    class RuntimeEvaluator {
       public:
        explicit RuntimeEvaluator(bool collectQasmLog = true) : m_collectQasmLog(collectQasmLog) {}
        void execute(Program& program);
        const std::unordered_map<const Expression*, std::vector<int>>& measurements() const {
            return m_measurements;
        }
        std::string getQasm() const { return m_sim.getQasm(); }

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
