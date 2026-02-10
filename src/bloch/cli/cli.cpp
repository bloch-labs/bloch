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

#include "bloch/cli/cli.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "bloch/compiler/import/module_loader.hpp"
#include "bloch/compiler/semantics/semantic_analyser.hpp"
#include "bloch/runtime/runtime_evaluator.hpp"
#include "bloch/support/error/bloch_error.hpp"
#include "bloch/update/update_manager.hpp"

namespace bloch::cli {
    namespace {
        namespace fs = std::filesystem;

        struct CliOption {
            std::string_view flag;
            std::string_view arg;
            std::string_view description;
        };

        static constexpr std::string_view kFlagHelp = "--help";
        static constexpr std::string_view kFlagVersion = "--version";
        static constexpr std::string_view kFlagEmitQasm = "--emit-qasm";
        static constexpr std::string_view kFlagShotsPrefix = "--shots=";
        static constexpr std::string_view kFlagEchoPrefix = "--echo=";
        static constexpr std::string_view kFlagUpdate = "--update";

        static constexpr std::array<CliOption, 6> kCliOptions = {
            CliOption{kFlagHelp, "", "Show this help and exit"},
            CliOption{kFlagVersion, "", "Print version and exit (checks for newer releases)"},
            CliOption{kFlagEmitQasm, "", "Print emitted QASM to stdout"},
            CliOption{
                "--shots", "=N",
                "Run the program N times and aggregate @tracked counts (deprecated in v2.0.0; "
                "prefer @shots(N))"},
            CliOption{"--echo", "=auto|all|none",
                      "Control echo statements (default: auto; suppress when taking many shots)"},
            CliOption{kFlagUpdate, "", "Download and install the latest release"},
        };

        std::string formattedVersion(const Context& ctx) {
            constexpr std::string_view unknown = "unknown";
            if (ctx.commit == unknown || ctx.commit.empty()) {
                return std::string(ctx.version);
            }
            return std::string(ctx.version) + " (" + std::string(ctx.commit) + ")";
        }

        void printHelp(const Context& ctx) {
            size_t width = 0;
            for (const auto& opt : kCliOptions) {
                width = std::max(width, opt.flag.size() + opt.arg.size());
            }
            std::cout << "Bloch " << formattedVersion(ctx) << "\n"
                      << "Usage: bloch [options] <file.bloch>\n\n"
                      << "Options:\n";
            for (const auto& opt : kCliOptions) {
                std::ostringstream line;
                line << "  " << opt.flag << opt.arg;
                std::cout << std::left << std::setw(static_cast<int>(width + 4)) << line.str()
                          << opt.description << "\n";
            }
            std::cout << "\nBehaviour:\n"
                      << "  - Writes <file>.qasm alongside the input file.\n"
                      << "  - When --shots is used, prints an aggregate table of tracked values.\n"
                      << std::endl;
        }

        void printVersion(const Context& ctx) { std::cout << formattedVersion(ctx) << std::endl; }

        std::string normaliseVersion(std::string_view version) {
            std::string v(version);
            if (!v.empty() && v.front() == 'v')
                v.erase(v.begin());
            return v;
        }

        void addPathCandidate(std::vector<std::string>& paths, const fs::path& candidate) {
            if (candidate.empty())
                return;
            std::error_code ec;
            fs::path canonical = fs::weakly_canonical(candidate, ec);
            std::string normalised = (ec ? candidate.lexically_normal() : canonical).string();
            if (normalised.empty())
                return;
            if (std::find(paths.begin(), paths.end(), normalised) == paths.end())
                paths.push_back(normalised);
        }

        void addVersionedRoots(std::vector<std::string>& paths, const fs::path& root,
                               const std::string& version) {
            if (root.empty())
                return;
            if (!version.empty()) {
                addPathCandidate(paths, root / version);
                addPathCandidate(paths, root / ("v" + version));
            }
            addPathCandidate(paths, root);
        }

        std::vector<std::string> resolveStdlibSearchPaths(const Context& ctx, const char* argv0) {
            std::vector<std::string> paths;
            std::string version = normaliseVersion(ctx.version);

            if (const char* overridePath = std::getenv("BLOCH_STDLIB_PATH");
                overridePath && *overridePath) {
                addVersionedRoots(paths, fs::path(overridePath), version);
            } else {
#if defined(_WIN32)
                if (const char* localAppData = std::getenv("LOCALAPPDATA");
                    localAppData && *localAppData) {
                    addVersionedRoots(paths, fs::path(localAppData) / "Bloch" / "library", version);
                }
#elif defined(__APPLE__)
                if (const char* home = std::getenv("HOME"); home && *home) {
                    addVersionedRoots(paths,
                                      fs::path(home) / "Library" / "Application Support" / "Bloch" /
                                          "library",
                                      version);
                }
#else
                fs::path dataRoot;
                if (const char* xdgDataHome = std::getenv("XDG_DATA_HOME");
                    xdgDataHome && *xdgDataHome) {
                    dataRoot = fs::path(xdgDataHome);
                } else if (const char* home = std::getenv("HOME"); home && *home) {
                    dataRoot = fs::path(home) / ".local" / "share";
                }
                if (!dataRoot.empty())
                    addVersionedRoots(paths, dataRoot / "bloch" / "library", version);
#endif
            }

            if (argv0 && *argv0) {
                std::error_code ec;
                fs::path exePath = fs::weakly_canonical(fs::path(argv0), ec);
                if (ec)
                    exePath = fs::absolute(fs::path(argv0), ec);
                if (!exePath.empty()) {
                    fs::path shareRoot = exePath.parent_path() / ".." / "share" / "bloch";
                    addVersionedRoots(paths, shareRoot / "library", version);
                    addPathCandidate(paths, shareRoot / "stdlib");
                    // Development tree fallback (e.g. ./build/bin/bloch from source checkout).
                    addPathCandidate(paths, exePath.parent_path() / ".." / ".." / "library");
                }
            }

            addPathCandidate(paths, fs::current_path() / "library");
            addPathCandidate(paths, fs::current_path() / "stdlib");

            return paths;
        }

        int runImpl(int argc, char** argv, const Context& ctx) {
            const std::string version(ctx.version);
            if (argc < 2) {
                std::cerr << "Usage: bloch [options] <file.bloch> (use --help for details)\n";
                return 1;
            }

            bool emitQasm = false;
            int shots = 1;
            bool shotsProvided = false;
            bool isCliShots = false;
            int cliShots = 1;
            bool isAnnotationShots = false;
            std::string echoOpt;
            std::string file;

            for (int i = 1; i < argc; ++i) {
                std::string arg = argv[i];
                if (arg == kFlagHelp) {
                    printHelp(ctx);
                    return 0;
                } else if (arg == kFlagVersion) {
                    printVersion(ctx);
                    bloch::update::checkForUpdatesIfDue(version);
                    return 0;
                } else if (arg == kFlagUpdate) {
                    return bloch::update::performSelfUpdate(version, argv[0]) ? 0 : 1;
                } else if (arg == kFlagEmitQasm) {
                    emitQasm = true;
                } else if (arg.rfind(kFlagShotsPrefix, 0) == 0) {
                    isCliShots = true;
                    cliShots = std::stoi(arg.substr(kFlagShotsPrefix.size()));
                    if (cliShots <= 0) {
                        std::cerr << "--shots must be positive\n";
                        return 1;
                    }
                } else if (arg.rfind(kFlagEchoPrefix, 0) == 0) {
                    echoOpt = arg.substr(kFlagEchoPrefix.size());
                } else {
                    file = arg;
                }
            }
            if (file.empty()) {
                std::cerr << "No input file provided (use --help for usage)\n";
                return 1;
            }

            // Run a non-blocking update check at most once every 72 hours.
            bloch::update::checkForUpdatesIfDue(version);

            try {
                auto stdlibPaths = resolveStdlibSearchPaths(ctx, argc > 0 ? argv[0] : nullptr);
                bloch::compiler::ModuleLoader loader(stdlibPaths);
                std::unique_ptr<bloch::compiler::Program> program = loader.load(file);
                bool isAnnotationShots = program->shots.first;

                if (isCliShots && !isAnnotationShots) {
                    shotsProvided = true;
                    shots = cliShots;
                    bloch::support::blochWarning(
                        0, 0,
                        "The '--shots=N' flag will be deprecated in v2.0.0. Please decorate your "
                        "main() function with the @shots(N) annotation instead.");
                } else if (!isCliShots && isAnnotationShots) {
                    shotsProvided = true;
                    shots = program->shots.second;
                } else if (isCliShots && isAnnotationShots) {
                    shotsProvided = true;
                    shots = program->shots.second;
                    if (cliShots != shots) {
                        bloch::support::blochWarning(
                            0, 0,
                            "The '--shots=N' flag differs from your @shots(N) annotation. Ignoring "
                            "CLI flag and using annotation value.");
                    }
                }

                // By default we suppress echo when taking many shots, unless the user
                // explicitly asks for it via --echo=all.
                bool echoAll =
                    echoOpt.empty() ? (!shotsProvided || shots == 1) : (echoOpt == "all");
                if (shotsProvided && shots > 1 && echoOpt.empty())
                    bloch::support::blochInfo(0, 0,
                                              "suppressing echo; to view them use --echo=all");

                bloch::compiler::SemanticAnalyser analyser;
                analyser.analyse(*program);
                std::string qasm;
                if (shotsProvided) {
                    // Multi-shot execution: aggregate tracked values and report a summary.
                    std::unordered_map<std::string, std::unordered_map<std::string, int>> aggregate;
                    auto start = std::chrono::steady_clock::now();
                    for (int s = 0; s < shots; ++s) {
                        bloch::runtime::RuntimeEvaluator evaluator(s == shots - 1);
                        evaluator.setEcho(echoAll);
                        // Suppress per-shot warnings; only show for last shot
                        if (s < shots - 1)
                            evaluator.setWarnOnExit(false);
                        evaluator.execute(*program);
                        if (s == shots - 1)
                            qasm = evaluator.getQasm();
                        for (const auto& vk : evaluator.trackedCounts())
                            for (const auto& vv : vk.second)
                                aggregate[vk.first][vv.first] += vv.second;
                    }
                    auto end = std::chrono::steady_clock::now();
                    double elapsed =
                        std::chrono::duration_cast<std::chrono::duration<double>>(end - start)
                            .count();
                    std::string base = file.substr(0, file.find_last_of('.'));
                    std::ofstream qfile(base + ".qasm");
                    qfile << qasm;
                    qfile.close();

                    // Warn if nothing was tracked, but still print run header and timing
                    if (aggregate.empty())
                        bloch::support::blochWarning(
                            0, 0, "No tracked variables. Use @tracked to collect statistics.");

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
                            std::stable_sort(
                                vals.begin(), vals.end(), [&](const auto& a, const auto& b) {
                                    bool ab = isBinary(a.first);
                                    bool bb = isBinary(b.first);
                                    if (ab != bb)
                                        return ab;  // binary outcomes first; e.g., place '?' at end
                                    if (!ab && !bb)
                                        return a.first < b.first;
                                    // Both binary: compare as integers, prefer shorter width first
                                    if (a.first.size() != b.first.size())
                                        return a.first.size() < b.first.size();
                                    return std::stoi(a.first, nullptr, 2) <
                                           std::stoi(b.first, nullptr, 2);
                                });
                            // Dynamic column sizing for outcome strings
                            size_t outcomeWidth = 7;
                            for (const auto& p : vals)
                                outcomeWidth = std::max(outcomeWidth, p.first.size());
                            std::cout << std::left << std::setw(static_cast<int>(outcomeWidth))
                                      << "outcome"
                                      << " | " << std::right << std::setw(5) << "count"
                                      << " | " << std::setw(5) << "prob"
                                      << "\n";
                            std::cout << std::string(outcomeWidth, '-') << "-+-------+-----\n";
                            for (auto& p : vals) {
                                double prob = static_cast<double>(p.second) / shots;
                                std::cout << std::left << std::setw(static_cast<int>(outcomeWidth))
                                          << p.first << " | " << std::right << std::setw(5)
                                          << p.second << " | " << std::setw(5) << prob << "\n";
                            }
                            std::cout << "\n";
                        }
                    }
                    if (emitQasm) {
                        std::cout << qasm;
                        return 0;
                    }
                } else {
                    bloch::runtime::RuntimeEvaluator evaluator;
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
                std::cerr << bloch::support::format(bloch::support::MessageLevel::Error, 0, 0,
                                                    "Stopping program execution...");
                std::cerr << ex.what() << std::endl;
                return 1;
            }
            return 0;
        }

    }  // namespace

    int run(int argc, char** argv, const Context& ctx) { return runImpl(argc, argv, ctx); }

}  // namespace bloch::cli
