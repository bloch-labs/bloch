#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "bloch/error/bloch_error.hpp"
#include "bloch/lexer/lexer.hpp"
#include "bloch/parser/parser.hpp"
#include "bloch/runtime/runtime_evaluator.hpp"
#include "bloch/semantics/semantic_analyser.hpp"

#ifndef BLOCH_VERSION
#define BLOCH_VERSION "dev"
#endif

static void printHelp() {
    std::cout << "Bloch " << BLOCH_VERSION << "\n"
              << "Usage: bloch [options] <file.bloch>\n\n"
              << "Options:\n"
              << "  --help          Show this help and exit\n"
              << "  --version       Print version and exit\n"
              << "  --emit-qasm     Print emitted QASM to stdout\n"
              << "  --shots=N       Run the program N times and aggregate @tracked counts\n"
              << "  --echo=all|none Control echo statements (default: auto)\n\n"
              << "Behavior:\n"
              << "  - Writes <file>.qasm alongside the input file.\n"
              << "  - When --shots is used, prints an aggregate table of tracked values.\n"
              << std::endl;
}

static void printVersion() { std::cout << BLOCH_VERSION << std::endl; }

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: bloch [options] <file.bloch> (use --help for details)\n";
        return 1;
    }
    // Flags:
    //  --emit-qasm  prints the QASM log after execution
    //  --shots=N    runs the program N times and aggregates @tracked counts
    //  --echo=all   echo statements are printed per shot
    //  --echo=none  no echo statements are printed
    //  --version    prints the build/version string and exits
    //  --help       prints usage information and exits
    // TODO: Add a --json flag
    bool emitQasm = false;
    int shots = 1;
    bool shotsProvided = false;
    std::string echoOpt;
    std::string file;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help") {
            printHelp();
            return 0;
        } else if (arg == "--version") {
            printVersion();
            return 0;
        } else if (arg == "--emit-qasm") {
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
        std::cerr << "No input file provided (use --help for usage)\n";
        return 1;
    }
    // By default we suppress echo when taking many shots, unless the user
    // explicitly asks for it via --echo=all.
    bool echoAll = echoOpt.empty() ? (!shotsProvided || shots == 1) : (echoOpt == "all");
    if (shotsProvided && shots > 1 && echoOpt.empty())
        bloch::blochInfo(0, 0, "suppressing echo; to view them use --echo=all");

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
            // Multi-shot execution: aggregate tracked values and report a summary.
            std::unordered_map<std::string, std::unordered_map<std::string, int>> aggregate;
            auto start = std::chrono::steady_clock::now();
            for (int s = 0; s < shots; ++s) {
                bloch::RuntimeEvaluator evaluator;
                evaluator.setEcho(echoAll);
                // Suppress per-shot warnings; only show for last shot
                if (s < shots - 1)
                    evaluator.setWarnOnExit(false);
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

            // Warn if nothing was tracked, but still print run header and timing
            if (aggregate.empty())
                bloch::blochWarning(0, 0,
                                    "No tracked variables. Use @tracked to collect statistics.");

            std::cout << "Shots: " << shots << "\n";
            std::cout << "Backend: Bloch Ideal Simulator\n";
            std::cout << std::fixed << std::setprecision(3);
            std::cout << "Elapsed: " << elapsed << "s\n\n";

            if (!aggregate.empty()) {
                for (auto& var : aggregate) {
                    // Header: e.g., "qubit q" or "qubit[] qreg"
                    std::cout << var.first << "\n";
                    std::vector<std::pair<std::string, int>> vals(var.second.begin(),
                                                                  var.second.end());
                    auto isBinary = [](const std::string& s) {
                        return !s.empty() && s.find_first_not_of("01") == std::string::npos;
                    };
                    std::stable_sort(vals.begin(), vals.end(), [&](const auto& a, const auto& b) {
                        bool ab = isBinary(a.first);
                        bool bb = isBinary(b.first);
                        if (ab != bb)
                            return ab;  // binary outcomes first; e.g., place '?' at end
                        if (!ab && !bb)
                            return a.first < b.first;
                        // Both binary: compare as integers, prefer shorter width first
                        if (a.first.size() != b.first.size())
                            return a.first.size() < b.first.size();
                        return std::stoi(a.first, nullptr, 2) < std::stoi(b.first, nullptr, 2);
                    });
                    // Dynamic column sizing for outcome strings
                    size_t outcomeWidth = 7;
                    for (const auto& p : vals)
                        outcomeWidth = std::max(outcomeWidth, p.first.size());
                    std::cout << std::left << std::setw(static_cast<int>(outcomeWidth)) << "outcome"
                              << " | " << std::right << std::setw(5) << "count"
                              << " | " << std::setw(5) << "prob"
                              << "\n";
                    std::cout << std::string(outcomeWidth, '-') << "-+-------+-----\n";
                    for (auto& p : vals) {
                        double prob = static_cast<double>(p.second) / shots;
                        std::cout << std::left << std::setw(static_cast<int>(outcomeWidth))
                                  << p.first << " | " << std::right << std::setw(5) << p.second
                                  << " | " << std::setw(5) << prob << "\n";
                    }
                    std::cout << "\n";
                }
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
        // Print a clear stop message, then the actual error
        std::cerr << bloch::format(bloch::MessageLevel::Error, 0, 0,
                                   "Stopping program execution...");
        std::cerr << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
