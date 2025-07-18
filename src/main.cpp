#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "bloch/codegen/cpp_generator.hpp"
#include "bloch/lexer/lexer.hpp"
#include "bloch/parser/parser.hpp"
#include "bloch/runtime/runtime_evaluator.hpp"
#include "bloch/semantics/semantic_analyser.hpp"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: bloch [--emit-qasm|--emit-cpp] <file.bloch>\n";
        return 1;
    }
    bool emitQasm = false;
    bool emitCpp = false;
    std::string file;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--emit-qasm")
            emitQasm = true;
        else if (arg == "--emit-cpp")
            emitCpp = true;
        else
            file = arg;
    }
    if (file.empty()) {
        std::cerr << "No input file provided\n";
        return 1;
    }
    std::ifstream in(file);
    if (!in) {
        std::cerr << "Failed to open " << file << "\n";
        return 1;
    }
    std::string src((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    try {
        bloch::Lexer lexer(src);
        auto tokens = lexer.tokenize();
        bloch::Parser parser(std::move(tokens));
        auto program = parser.parse();
        bloch::SemanticAnalyser analyser;
        analyser.analyse(*program);
        bloch::RuntimeEvaluator evaluator;
        evaluator.execute(*program);
        std::string qasm = evaluator.getQasm();
        bloch::CppGenerator gen(evaluator.measurements());
        std::string cpp = gen.generate(*program);
        std::string base = file.substr(0, file.find_last_of('.'));
        std::ofstream qfile(base + ".qasm");
        qfile << qasm;
        qfile.close();
        std::ofstream cfile(base + ".cpp");
        cfile << cpp;
        cfile.close();
        if (emitQasm) {
            std::cout << qasm;
            return 0;
        }
        if (emitCpp) {
            std::cout << cpp;
            return 0;
        }
        std::string out = base + ".out";
        std::string cmd = "g++ -std=c++17 " + base + ".cpp -o " + out;
        if (std::system(cmd.c_str()) != 0) {
            std::cerr << "Failed to compile generated C++\n";
            return 1;
        }
        std::system(("./" + out).c_str());
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
    return 0;
}