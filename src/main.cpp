#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "bloch/lexer/lexer.hpp"
#include "bloch/parser/parser.hpp"
#include "bloch/runtime/runtime_evaluator.hpp"
#include "bloch/semantics/semantic_analyser.hpp"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: bloch [--emit-qasm] [--shots=N] [--echo=all|none] <file.bloch>\n";
        return 1;
    }
    bool emitQasm = false;
    int shots = 1;
    bool shotsProvided = false;
    std::string echoOpt;
    std::string file;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--emit-qasm") {
            emitQasm = true;
        } else if (arg.rfind("--shots=", 0) == 0) {
            shotsProvided = true;
            shots = std::stoi(arg.substr(8));
            if (shots <= 0) {
                std::cerr << "--shots must be positive\n";
                return 1;
            }
        } else if (arg.rfind("--echo=", 0) == 0) {
            echoOpt = arg.substr(7);
        } else {
            file = arg;
        }
    }
    if (file.empty()) {
        std::cerr << "No input file provided\n";
        return 1;
    }
    bool echoAll = echoOpt.empty() ? (!shotsProvided || shots == 1) : (echoOpt == "all");
    if (shotsProvided && shots > 1 && echoOpt.empty())
        std::clog << "[INFO] suppressing echo; to view them use --echo=all\n";

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
        std::string qasm;
        if (shotsProvided) {
            std::unordered_map<std::string, std::unordered_map<std::string, int>> aggregate;
            auto start = std::chrono::steady_clock::now();
            for (int s = 0; s < shots; ++s) {
                bloch::RuntimeEvaluator evaluator;
                evaluator.setEcho(echoAll);
                evaluator.execute(*program);
                if (s == shots - 1)
                    qasm = evaluator.getQasm();
                for (const auto& vk : evaluator.trackedCounts())
                    for (const auto& vv : vk.second) aggregate[vk.first][vv.first] += vv.second;
            }
            auto end = std::chrono::steady_clock::now();
            double elapsed =
                std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
            std::string base = file.substr(0, file.find_last_of('.'));
            std::ofstream qfile(base + ".qasm");
            qfile << qasm;
            qfile.close();
            if (!aggregate.empty()) {
                std::cout << "Shots: " << shots << "\n";
                std::cout << "Backend: qasm_simulator\n";
                std::cout << std::fixed << std::setprecision(3);
                std::cout << "Elapsed: " << elapsed << "s\n\n";
                for (auto& var : aggregate) {
                    std::cout << var.first << "\n";
                    std::vector<std::pair<std::string, int>> vals(var.second.begin(),
                                                                  var.second.end());
                    std::sort(vals.begin(), vals.end(), [](const auto& a, const auto& b) {
                        if (a.second != b.second)
                            return a.second > b.second;
                        return a.first < b.first;
                    });
                    std::cout << " value | count | prob\n";
                    std::cout << "---------------------\n";
                    for (auto& p : vals) {
                        double prob = static_cast<double>(p.second) / shots;
                        std::cout << std::setw(6) << p.first << " | " << std::setw(5) << p.second
                                  << " | " << std::setw(5) << prob << "\n";
                    }
                    std::cout << "\n";
                }
            } else {
                std::cerr << "No tracked variables. Use @tracked to collect statistics.\n";
            }
            if (emitQasm) {
                std::cout << qasm;
                return 0;
            }
        } else {
            bloch::RuntimeEvaluator evaluator;
            evaluator.setEcho(echoAll);
            evaluator.execute(*program);
            qasm = evaluator.getQasm();
            std::string base = file.substr(0, file.find_last_of('.'));
            std::ofstream qfile(base + ".qasm");
            qfile << qasm;
            qfile.close();
            if (emitQasm) {
                std::cout << qasm;
                return 0;
            }
        }
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
