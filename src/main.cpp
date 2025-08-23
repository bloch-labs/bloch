#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "bloch/lexer/lexer.hpp"
#include "bloch/parser/parser.hpp"
#include "bloch/runtime/runtime_evaluator.hpp"
#include "bloch/semantics/semantic_analyser.hpp"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: bloch [--emit-qasm] <file.bloch>\n";
        return 1;
    }
    bool emitQasm = false;
    std::string file;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--emit-qasm")
            emitQasm = true;
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
        std::string base = file.substr(0, file.find_last_of('.'));
        std::ofstream qfile(base + ".qasm");
        qfile << qasm;
        qfile.close();
        if (emitQasm) {
            std::cout << qasm;
            return 0;
        }
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
    return 0;
}