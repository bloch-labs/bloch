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

#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "bloch/core/import/module_loader.hpp"
#include "bloch/core/lexer/lexer.hpp"
#include "bloch/core/parser/parser.hpp"
#include "bloch/core/semantics/semantic_analyser.hpp"
#include "bloch/runtime/qasm_simulator.hpp"
#include "bloch/runtime/runtime_evaluator.hpp"
#include "bloch/support/error/bloch_error.hpp"
#include "test_framework.hpp"

using namespace bloch::core;
using namespace bloch::runtime;
using bloch::support::BlochError;

static std::unique_ptr<Program> parseProgram(const char* src) {
    Lexer lexer(src);
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    return parser.parse();
}

static std::filesystem::path makeTempDir(const std::string& name) {
    auto base = std::filesystem::temp_directory_path() /
                ("bloch_import_" + name + "_" +
                 std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::create_directories(base);
    return base;
}

static void writeFile(const std::filesystem::path& path, const std::string& contents) {
    std::ofstream out(path);
    out << contents;
    out.close();
}

TEST(QasmSimulatorTest, AllocatesPowersOfTwoStateSize) {
    QasmSimulator sim;
    EXPECT_EQ(sim.stateSize(), 1u);
    int q0 = sim.allocateQubit();
    EXPECT_EQ(q0, 0);
    EXPECT_EQ(sim.stateSize(), 2u);
    int q1 = sim.allocateQubit();
    EXPECT_EQ(q1, 1);
    EXPECT_EQ(sim.stateSize(), 4u);
}

TEST(QasmSimulatorTest, LoggingCanBeSuppressedPerInstance) {
    QasmSimulator sim(false);
    sim.allocateQubit();
    sim.h(0);
    std::string qasm = sim.getQasm();
    EXPECT_NE(qasm.find("qreg q[1]"), std::string::npos);
    EXPECT_EQ(qasm.find("h q[0]"), std::string::npos);

    QasmSimulator sim2(true);
    sim2.allocateQubit();
    sim2.x(0);
    qasm = sim2.getQasm();
    EXPECT_NE(qasm.find("x q[0]"), std::string::npos);
}

TEST(QasmSimulatorTest, CxOnlyActsWhenControlIsOne) {
    {
        QasmSimulator sim;
        int c = sim.allocateQubit();
        int t = sim.allocateQubit();
        sim.cx(c, t);  // control remains |0>
        EXPECT_EQ(sim.measure(t), 0);
    }
    {
        QasmSimulator sim;
        int c = sim.allocateQubit();
        int t = sim.allocateQubit();
        sim.x(c);      // prepare |10>
        sim.cx(c, t);  // should flip target to 1
        EXPECT_EQ(sim.measure(t), 1);
        EXPECT_EQ(sim.measure(c), 1);
    }
    {
        QasmSimulator sim;
        int q0 = sim.allocateQubit();
        int q1 = sim.allocateQubit();
        sim.x(q0);
        sim.x(q1);       // prepare |11>
        sim.cx(q1, q0);  // should flip q0 to 0
        EXPECT_EQ(sim.measure(q0), 0);
        EXPECT_EQ(sim.measure(q1), 1);
    }
}

TEST(RuntimeTest, ParenthesisedExpressionsEvaluate) {
    const char* src =
        "function main() -> void { for (int i = 0; i < 6; i = i + 1) { echo((i + 1) % 6); } }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    std::ostringstream output;
    auto* oldBuf = std::cout.rdbuf(output.rdbuf());
    eval.execute(*program);
    std::cout.rdbuf(oldBuf);
    EXPECT_EQ("1\n2\n3\n4\n5\n0\n", output.str());
}

TEST(RuntimeTest, GeneratesQasm) {
    const char* src =
        "@quantum function flip() -> bit { qubit q; h(q); bit r = measure q; return r; } function "
        "main() -> void { bit b = flip(); }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    eval.execute(*program);
    std::string qasm = eval.getQasm();
    EXPECT_NE(qasm.find("OPENQASM 2.0"), std::string::npos);
    EXPECT_NE(qasm.find("h q[0]"), std::string::npos);
    EXPECT_NE(qasm.find("measure q[0]"), std::string::npos);
}

TEST(RuntimeTest, MultipleQubitDeclarationsAllocateDistinctQubits) {
    const char* src = "function main() -> void { qubit q0, q1; h(q0); h(q1); }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    eval.execute(*program);
    std::string qasm = eval.getQasm();
    EXPECT_NE(qasm.find("h q[0]"), std::string::npos);
    EXPECT_NE(qasm.find("h q[1]"), std::string::npos);
}

TEST(RuntimeTest, MeasurementsPreservedInForLoops) {
    const char* src =
        "@quantum function flip() -> bit { qubit q; x(q); bit r = measure q; return r; } "
        "function main() -> void { for (int i = 0; i < 3; i = i + 1) { bit b = flip(); } }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    eval.execute(*program);
    const auto& meas = eval.measurements();
    ASSERT_EQ(meas.size(), 2u);
    for (const auto& kv : meas) {
        ASSERT_EQ(kv.second.size(), 3u);
    }
}

TEST(RuntimeTest, MeasurementsPreservedInWhileLoops) {
    const char* src =
        "@quantum function flip() -> bit { qubit q; x(q); bit r = measure q; return r; } "
        "function main() -> void { int i = 0; while (i < 3) { bit b = flip(); i = i + 1; } }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    eval.execute(*program);
    const auto& meas = eval.measurements();
    ASSERT_EQ(meas.size(), 2u);
    for (const auto& kv : meas) {
        ASSERT_EQ(kv.second.size(), 3u);
    }
}

TEST(RuntimeTest, EchoConcatenatesValues) {
    const char* src =
        "function main() -> void { bit b = 1b; echo(\"Measured: \" + b); echo(5 + 5); }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    std::ostringstream output;
    auto* oldBuf = std::cout.rdbuf(output.rdbuf());
    eval.execute(*program);
    std::cout.rdbuf(oldBuf);
    EXPECT_EQ("Measured: 1\n10\n", output.str());
}

TEST(RuntimeTest, EchoFloatPrintsWithDecimal) {
    const char* src = "function main() -> void { float f = 3f; echo(f); }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    std::ostringstream output;
    auto* oldBuf = std::cout.rdbuf(output.rdbuf());
    eval.execute(*program);
    std::cout.rdbuf(oldBuf);
    EXPECT_EQ("3.0\n", output.str());
}

TEST(RuntimeTest, NullEqualityChecksObjectPresence) {
    const char* src =
        "class Foo { public constructor() -> Foo = default; } function main() -> void { Foo f = "
        "null; echo(f == null); Foo g = new Foo(); echo(g == null); }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    std::ostringstream output;
    auto* oldBuf = std::cout.rdbuf(output.rdbuf());
    eval.execute(*program);
    std::cout.rdbuf(oldBuf);
    EXPECT_EQ("1\n0\n", output.str());
}

TEST(RuntimeTest, NullInequalityChecksObjectPresence) {
    const char* src =
        "class Foo { public constructor() -> Foo = default; } function main() -> void { Foo f = "
        "null; echo(f != null); Foo g = new Foo(); echo(g != null); }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    std::ostringstream output;
    auto* oldBuf = std::cout.rdbuf(output.rdbuf());
    eval.execute(*program);
    std::cout.rdbuf(oldBuf);
    EXPECT_EQ("0\n1\n", output.str());
}

TEST(RuntimeTest, MethodOverloadDispatchesByParameterTypes) {
    const char* src =
        "class Foo { public function val(int x) -> int { return 1; } public function val(float x) "
        "-> int { return 2; } public constructor() -> Foo = default; } "
        "function main() -> void { Foo f = new Foo(); echo(f.val(1)); echo(f.val(1f)); }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    std::ostringstream output;
    auto* oldBuf = std::cout.rdbuf(output.rdbuf());
    eval.execute(*program);
    std::cout.rdbuf(oldBuf);
    EXPECT_EQ("1\n2\n", output.str());
}

TEST(RuntimeTest, MemberAccessOnNullThrows) {
    const char* src =
        "class Foo { public int x; public constructor() -> Foo = default; } function main() -> "
        "void { Foo f = null; int y = f.x; }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    EXPECT_THROW(eval.execute(*program), BlochError);
}

TEST(RuntimeTest, SkipsGcThreadWhenProgramHasNoClasses) {
    const char* src = "function main() -> void { bit b = 0b; echo(b); }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    eval.execute(*program);
    EXPECT_FALSE(eval.gcThreadStartedForTest());
}

TEST(RuntimeTest, StartsGcThreadWhenClassesExist) {
    const char* src =
        "class Foo { public constructor() -> Foo = default; } function main() -> void { Foo f = "
        "new Foo(); }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    eval.execute(*program);
    EXPECT_TRUE(eval.gcThreadStartedForTest());
}

TEST(RuntimeTest, IntDivisionPromotesToFloat) {
    const char* src = "function main() -> void { echo(1/2); }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    std::ostringstream output;
    auto* oldBuf = std::cout.rdbuf(output.rdbuf());
    eval.execute(*program);
    std::cout.rdbuf(oldBuf);
    EXPECT_EQ("0.5\n", output.str());
}

TEST(RuntimeTest, TernaryExecutesCorrectBranch) {
    const char* src =
        "function main() -> void { int x = 0; x ? echo(\"true\"); : echo(\"false\"); }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    std::ostringstream output;
    auto* oldBuf = std::cout.rdbuf(output.rdbuf());
    eval.execute(*program);
    std::cout.rdbuf(oldBuf);
    EXPECT_EQ("false\n", output.str());
}

TEST(RuntimeTest, PostIncrementAndDecrement) {
    const char* src =
        "function main() -> void { int a = 5; echo(a++); echo(a); echo(a--); echo(a); }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    std::ostringstream output;
    auto* oldBuf = std::cout.rdbuf(output.rdbuf());
    eval.execute(*program);
    std::cout.rdbuf(oldBuf);
    EXPECT_EQ("5\n6\n6\n5\n", output.str());
}

TEST(RuntimeTest, TracksVariableSingleShot) {
    const char* src = "function main() -> void { @tracked qubit q; x(q); }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    eval.execute(*program);
    const auto& counts = eval.trackedCounts();
    ASSERT_EQ(counts.at("qubit q").at("?"), 1);
}

TEST(RuntimeTest, TracksVariableMultipleShots) {
    const char* src = "function main() -> void { @tracked qubit q; x(q); }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    std::unordered_map<std::string, std::unordered_map<std::string, int>> agg;
    for (int i = 0; i < 5; ++i) {
        RuntimeEvaluator eval;
        eval.execute(*program);
        for (const auto& vk : eval.trackedCounts())
            for (const auto& vv : vk.second) agg[vk.first][vv.first] += vv.second;
    }
    ASSERT_EQ(agg["qubit q"]["?"], 5);
}

TEST(RuntimeTest, ExecuteIsSingleUse) {
    const char* src = "function main() -> void { qubit q; x(q); }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    eval.execute(*program);
    EXPECT_THROW(eval.execute(*program), BlochError);
}

TEST(RuntimeTest, EchoModes) {
    const char* src = "function main() -> void { echo(1); }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    eval.setEcho(false);
    std::ostringstream out;
    auto* old = std::cout.rdbuf(out.rdbuf());
    eval.execute(*program);
    std::cout.rdbuf(old);
    EXPECT_EQ("", out.str());
    RuntimeEvaluator eval2;
    eval2.setEcho(true);
    out.str("");
    old = std::cout.rdbuf(out.rdbuf());
    eval2.execute(*program);
    std::cout.rdbuf(old);
    EXPECT_EQ("1\n", out.str());
}

TEST(RuntimeTest, GenericInstantiationSpecialisesAtRuntime) {
    const char* src =
        "class Box<T> { public T v; public constructor(T v) -> Box<T> { this.v = v; return this; } "
        "public function get() -> T { return this.v; } } "
        "function main() -> void { Box<int> b = new Box<int>(1); echo(b.get()); }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    std::ostringstream out;
    auto* old = std::cout.rdbuf(out.rdbuf());
    eval.execute(*program);
    std::cout.rdbuf(old);
    EXPECT_EQ("1\n", out.str());
}

TEST(RuntimeTest, UnmeasuredTrackedQubit) {
    const char* src = "function main() -> void { @tracked qubit q; }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    eval.execute(*program);
    const auto& counts = eval.trackedCounts();
    ASSERT_EQ(counts.at("qubit q").at("?"), 1);
}

TEST(RuntimeTest, MeasuredTrackedQubit) {
    const char* src = "function main() -> void { @tracked qubit q; x(q); measure q; }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    eval.execute(*program);
    const auto& counts = eval.trackedCounts();
    ASSERT_EQ(counts.at("qubit q").at("1"), 1);
}

TEST(RuntimeTest, MeasureQubitArrayMarksAllMeasured) {
    const char* src = "function main() -> void { qubit[2] q; measure q; h(q[0]); h(q[1]); }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    EXPECT_THROW(eval.execute(*program), BlochError);
}

TEST(RuntimeTest, GateAfterMeasurementThrows) {
    const char* src = "function main() -> void { qubit q; measure q; h(q); }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    EXPECT_THROW(eval.execute(*program), BlochError);
}

TEST(RuntimeTest, GateAfterMeasurementReportsLocation) {
    const char* src =
        "function main() -> void {\n"
        "    qubit q;\n"
        "    measure q;\n"
        "    h(q);\n"
        "}";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    bool caught = false;
    try {
        eval.execute(*program);
    } catch (const BlochError& err) {
        caught = true;
        EXPECT_EQ(err.line, 4);
        EXPECT_EQ(err.column, 5);
    }
    EXPECT_TRUE(caught);
}

TEST(RuntimeTest, ResetAfterMeasurementUnblocksQubit) {
    const char* src = "function main() -> void { qubit q; measure q; reset q; x(q); }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    EXPECT_NO_THROW(eval.execute(*program));
    std::string qasm = eval.getQasm();
    EXPECT_NE(qasm.find("reset q[0]"), std::string::npos);
    EXPECT_NE(qasm.find("x q[0]"), std::string::npos);
}

TEST(RuntimeTest, MeasureExpressionAfterMeasurementThrows) {
    const char* src = "function main() -> void { qubit q; bit a = measure q; bit b = measure q; }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    EXPECT_THROW(eval.execute(*program), BlochError);
}

TEST(RuntimeTest, ResetClearsQubit) {
    const char* src =
        "function main() -> bit { qubit q; x(q); reset q; bit r = measure q; return r; }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    eval.execute(*program);
    const auto& meas = eval.measurements();
    ASSERT_EQ(meas.size(), 1u);
    EXPECT_EQ(meas.begin()->second[0], 0);
}

TEST(RuntimeTest, LogicalAndBitwiseOperations) {
    const char* src =
        "function main() -> void { echo(1b & 0b); echo(1b | 0b); echo(1b ^ 1b); echo(~0b); "
        "echo(!1b); echo(1b && 0b); echo(0b || 1b); }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    std::ostringstream output;
    auto* oldBuf = std::cout.rdbuf(output.rdbuf());
    eval.execute(*program);
    std::cout.rdbuf(oldBuf);
    EXPECT_EQ("0\n1\n0\n1\n0\n0\n1\n", output.str());
}

TEST(RuntimeTest, BitArrayBitwiseOperations) {
    const char* src =
        "function main() -> void { bit[] a = {0b, 1b, 1b, 0b}; bit[] b = {1b, 0b, 1b, 0b}; "
        "echo(~a); echo(a & b); echo(a | b); echo(a ^ b); }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    std::ostringstream output;
    auto* oldBuf = std::cout.rdbuf(output.rdbuf());
    eval.execute(*program);
    std::cout.rdbuf(oldBuf);
    EXPECT_EQ("{1, 0, 0, 1}\n{0, 0, 1, 0}\n{1, 1, 1, 0}\n{1, 1, 0, 0}\n", output.str());
}

TEST(RuntimeTest, IntArrayInitializationAndIndexing) {
    const char* src = "function main() -> void { int[] a = {0,1,2,3}; echo(a[0]); echo(a[3]); }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    std::ostringstream output;
    auto* oldBuf = std::cout.rdbuf(output.rdbuf());
    eval.execute(*program);
    std::cout.rdbuf(oldBuf);
    EXPECT_EQ("0\n3\n", output.str());
}

TEST(RuntimeTest, IntArraySizedDefaults) {
    const char* src = "function main() -> void { int[3] a; echo(a[0]); echo(a[2]); }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    std::ostringstream output;
    auto* oldBuf = std::cout.rdbuf(output.rdbuf());
    eval.execute(*program);
    std::cout.rdbuf(oldBuf);
    EXPECT_EQ("0\n0\n", output.str());
}

TEST(RuntimeTest, ArrayAssignmentHappyPaths) {
    const char* src =
        "function main() -> void { int[] a = {0,1}; a[0] = 2; a[1] = 1b; echo(a[0]); echo(a[1]); }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    std::ostringstream output;
    auto* old = std::cout.rdbuf(output.rdbuf());
    eval.execute(*program);
    std::cout.rdbuf(old);
    EXPECT_EQ("2\n1\n", output.str());
}

TEST(RuntimeTest, ArrayAssignmentTypeMismatchThrows) {
    const char* src = "function main() -> void { string[] s = {\"a\"}; s[0] = 1; }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    EXPECT_THROW(eval.execute(*program), BlochError);
}

TEST(RuntimeTest, ArrayAssignmentOutOfBoundsThrows) {
    const char* src = "function main() -> void { int[1] a; a[1] = 5; }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    EXPECT_THROW(eval.execute(*program), BlochError);
}

TEST(RuntimeTest, NegativeIndexRuntimeThrows) {
    const char* src = "function main() -> void { int[] a = {1,2}; int i = -1; echo(a[i]); }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    EXPECT_THROW(eval.execute(*program), BlochError);
}

TEST(RuntimeTest, UnaryTildeOnIntThrows) {
    const char* src = "function main() -> void { int x = 2; echo(~x); }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    EXPECT_THROW(eval.execute(*program), BlochError);
}

TEST(RuntimeTest, RyRzAppearInQasm) {
    const char* src = "function main() -> void { qubit q; ry(q, 1.0f); rz(q, 0.5f); }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    eval.execute(*program);
    std::string qasm = eval.getQasm();
    EXPECT_NE(qasm.find("ry("), std::string::npos);
    EXPECT_NE(qasm.find("rz("), std::string::npos);
}

TEST(RuntimeTest, ClassCtorDtorVirtualAndStatic) {
    const char* src = R"(
class Base {
    public constructor() -> Base { echo("Base::ctor"); }
    destructor() -> void { echo("Base::dtor"); }
    public virtual function name() -> string { return "Base"; }
    public function baseOnly() -> string { return "BaseOnly"; }
}

class Derived extends Base {
    public static int count;
    public constructor() -> Derived { Derived.count = Derived.count + 1; echo("Derived::ctor"); }
    destructor() -> void { echo("Derived::dtor"); }
    public override function name() -> string { return "Derived"; }
}

function main() -> void {
    Derived b = new Derived();
    echo(b.name());
    echo(b.baseOnly());
    echo(Derived.count);
    destroy b;
}
)";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    std::ostringstream output;
    auto* oldBuf = std::cout.rdbuf(output.rdbuf());
    eval.execute(*program);
    std::cout.rdbuf(oldBuf);
    EXPECT_EQ("Base::ctor\nDerived::ctor\nDerived\nBaseOnly\n1\nDerived::dtor\nBase::dtor\n",
              output.str());
}

TEST(RuntimeTest, ConstructorChainCallsExplicitSuperWithArgs) {
    const char* src = R"(
class Base {
    public int a = 1;
    public constructor(int v) -> Base { this.a = v; }
}

class Mid extends Base {
    public int m = 2;
    public constructor(int v) -> Mid { super(v + 1); this.m = this.a + 1; }
}

class Derived extends Mid {
    public int d = 3;
    public constructor() -> Derived { super(4); this.d = this.m + 1; }
    public function dump() -> void { echo(this.a); echo(this.m); echo(this.d); }
}

function main() -> void {
    Derived x = new Derived();
    x.dump();
}
)";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    std::ostringstream output;
    auto* oldBuf = std::cout.rdbuf(output.rdbuf());
    eval.execute(*program);
    std::cout.rdbuf(oldBuf);
    EXPECT_EQ("5\n6\n7\n", output.str());
}

TEST(RuntimeTest, SelectsConstructorByArgumentTypes) {
    const char* src = R"(
class Example {
    public int which = 0;
    public constructor(int _) -> Example { this.which = 1; }
    public constructor(string _) -> Example { this.which = 2; }
}

function main() -> void {
    Example e = new Example("hi");
    echo(e.which);
}
)";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    std::ostringstream output;
    auto* oldBuf = std::cout.rdbuf(output.rdbuf());
    eval.execute(*program);
    std::cout.rdbuf(oldBuf);
    EXPECT_EQ("2\n", output.str());
}

TEST(RuntimeTest, CycleCollectorReclaimsClassicalCycle) {
    const char* src =
        "class Node { public Node next; public constructor() -> Node { } } function main() -> void "
        "{ Node "
        "a = new "
        "Node(); Node b = new Node(); a.next = b; b.next = a; }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    eval.execute(*program);
    EXPECT_EQ(eval.heapObjectCount(), 0u);
}

TEST(RuntimeTest, CycleCollectorSkipsTrackedCycles) {
    const char* src =
        "class Q { @tracked qubit q; public Q other; public constructor() -> Q { } } function "
        "main() -> "
        "void { Q "
        "a = new Q(); Q b = new Q(); a.other = b; b.other = a; }";
    auto program = parseProgram(src);
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    eval.execute(*program);
    EXPECT_GE(eval.heapObjectCount(), 2u);
}

TEST(RuntimeTest, ImportsPublicClassAcrossModules) {
    auto dir = makeTempDir("basic");
    writeFile(dir / "Greeter.bloch",
              "class Greeter { public constructor() -> Greeter = default; public function "
              "hello() -> string { return \"hello\"; } }\n");
    writeFile(dir / "main.bloch",
              "import Greeter; function main() -> void { Greeter g = new Greeter(); "
              "echo(g.hello()); }\n");

    ModuleLoader loader({dir.string()});
    auto program = loader.load((dir / "main.bloch").string());
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    std::ostringstream output;
    auto* oldBuf = std::cout.rdbuf(output.rdbuf());
    eval.execute(*program);
    std::cout.rdbuf(oldBuf);
    std::filesystem::remove_all(dir);
    EXPECT_EQ("hello\n", output.str());
}

TEST(RuntimeTest, DottedImportResolvesNestedPath) {
    auto dir = makeTempDir("nested");
    std::filesystem::create_directories(dir / "pkg");
    writeFile(dir / "pkg" / "Utils.bloch",
              "class Utils { public constructor() -> Utils = default; public function value() -> "
              "int { return 7; } }\n");
    writeFile(dir / "main.bloch",
              "import pkg.Utils; function main() -> void { Utils u = new Utils(); echo(u.value()); "
              "}\n");

    ModuleLoader loader({dir.string()});
    auto program = loader.load((dir / "main.bloch").string());
    SemanticAnalyser analyser;
    analyser.analyse(*program);
    RuntimeEvaluator eval;
    std::ostringstream output;
    auto* oldBuf = std::cout.rdbuf(output.rdbuf());
    eval.execute(*program);
    std::cout.rdbuf(oldBuf);
    std::filesystem::remove_all(dir);
    EXPECT_EQ("7\n", output.str());
}

TEST(RuntimeTest, ImportRespectsPrivateMembersAcrossModules) {
    auto dir = makeTempDir("visibility");
    writeFile(dir / "Secret.bloch",
              "class Secret { public constructor() -> Secret = default; private function hidden() "
              "-> void { } public function expose() -> void { hidden(); } }\n");
    writeFile(dir / "main.bloch",
              "import Secret; function main() -> void { Secret s = new Secret(); s.hidden(); }\n");

    ModuleLoader loader({dir.string()});
    auto program = loader.load((dir / "main.bloch").string());
    SemanticAnalyser analyser;
    EXPECT_THROW(analyser.analyse(*program), BlochError);
    std::filesystem::remove_all(dir);
}

TEST(RuntimeTest, ImportCycleFailsWithDiagnostic) {
    auto dir = makeTempDir("cycle");
    writeFile(dir / "A.bloch", "import B; function main() -> void { }\n");
    writeFile(dir / "B.bloch", "import A; \n");

    ModuleLoader loader({dir.string()});
    EXPECT_THROW(loader.load((dir / "A.bloch").string()), BlochError);
    std::filesystem::remove_all(dir);
}

TEST(RuntimeTest, MultipleMainAcrossModulesFails) {
    auto dir = makeTempDir("multi_main");
    writeFile(dir / "Lib.bloch", "function main() -> void { }\n");
    writeFile(dir / "App.bloch", "import Lib; function main() -> void { }\n");

    ModuleLoader loader({dir.string()});
    EXPECT_THROW(loader.load((dir / "App.bloch").string()), BlochError);
    std::filesystem::remove_all(dir);
}
