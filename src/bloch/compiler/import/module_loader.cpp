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

#include "bloch/compiler/import/module_loader.hpp"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "bloch/compiler/lexer/lexer.hpp"
#include "bloch/compiler/parser/parser.hpp"
#include "bloch/support/error/bloch_error.hpp"

namespace bloch::compiler {

    using support::BlochError;
    using support::ErrorCategory;
    namespace fs = std::filesystem;

    ModuleLoader::ModuleLoader(std::vector<std::string> searchPaths)
        : m_searchPaths(std::move(searchPaths)) {
        auto blochPath = splitPathListEnv("BLOCH_PATH");
        m_searchPaths.insert(m_searchPaths.end(), blochPath.begin(), blochPath.end());

        auto stdlibOverride = splitPathListEnv("BLOCH_STDLIB_PATH");
        m_stdlibSearchPaths.insert(m_stdlibSearchPaths.end(), stdlibOverride.begin(),
                                   stdlibOverride.end());

        if (auto source = sourceStdlibPath(); !source.empty())
            m_stdlibSearchPaths.push_back(source);
        if (auto install = installStdlibPath(); !install.empty())
            m_stdlibSearchPaths.push_back(install);
        auto dataPaths = dataStdlibPaths();
        m_stdlibSearchPaths.insert(m_stdlibSearchPaths.end(), dataPaths.begin(), dataPaths.end());
    }

    std::string ModuleLoader::joinQualified(const std::vector<std::string>& parts) {
        std::ostringstream oss;
        for (size_t i = 0; i < parts.size(); ++i) {
            if (i)
                oss << ".";
            oss << parts[i];
        }
        return oss.str();
    }

    std::string ModuleLoader::canonicalize(const std::string& path) const {
        std::error_code ec;
        fs::path p(path);
        fs::path abs = fs::absolute(p, ec);
        if (!ec) {
            fs::path canon = fs::weakly_canonical(abs, ec);
            if (!ec)
                return canon.string();
        }
        return p.lexically_normal().string();
    }

    std::unique_ptr<Program> ModuleLoader::parseFile(const std::string& path) const {
        std::ifstream in(path);
        if (!in) {
            throw BlochError(ErrorCategory::Parse, 0, 0, "failed to open '" + path + "'");
        }
        std::string src((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        Lexer lexer(src);
        std::vector<Token> tokens = lexer.tokenize();
        Parser parser(std::move(tokens));
        return parser.parse();
    }

    std::string ModuleLoader::resolveImportPath(const std::vector<std::string>& parts,
                                                const std::string& fromDir) const {
        fs::path relative;
        for (const auto& p : parts) relative /= p;
        relative += ".bloch";

        // Search order:
        // 1) importing file's directory
        // 2) user-specified paths (ctor + BLOCH_PATH)
        // 3) current working directory
        // 4) stdlib paths (BLOCH_STDLIB_PATH, source tree, install tree, data dirs)
        std::vector<fs::path> bases;
        bases.push_back(fromDir);
        bases.push_back(fs::current_path());
        for (const auto& p : m_searchPaths) bases.emplace_back(p);
        for (const auto& p : m_stdlibSearchPaths) bases.emplace_back(p);

        for (const auto& base : bases) {
            std::error_code ec;
            fs::path candidate = fs::weakly_canonical(base / relative, ec);
            if (ec)
                continue;
            if (fs::exists(candidate, ec) && fs::is_regular_file(candidate, ec))
                return candidate.string();
        }
        return "";
    }

    std::vector<std::string> ModuleLoader::splitPathListEnv(const char* envVar) {
        std::vector<std::string> paths;
        const char* raw = std::getenv(envVar);
        if (!raw || *raw == '\0')
            return paths;

#ifdef _WIN32
        const char delimiter = ';';
#else
        const char delimiter = ':';
#endif
        std::stringstream ss(raw);
        std::string item;
        while (std::getline(ss, item, delimiter)) {
            if (!item.empty())
                paths.push_back(item);
        }
        return paths;
    }

    std::vector<std::string> ModuleLoader::dataStdlibPaths() {
        std::vector<std::string> paths;
        const char* xdg = std::getenv("XDG_DATA_HOME");
        if (xdg && *xdg) {
            paths.emplace_back(std::string(xdg) + "/bloch/library");
        } else {
            const char* home = std::getenv("HOME");
            if (home && *home)
                paths.emplace_back(std::string(home) + "/.local/share/bloch/library");
        }
        paths.emplace_back("/usr/local/share/bloch/library");
        paths.emplace_back("/usr/share/bloch/library");
        return paths;
    }

    std::string ModuleLoader::installStdlibPath() {
#ifdef BLOCH_STDLIB_INSTALL_DIR
        return std::string(BLOCH_STDLIB_INSTALL_DIR);
#else
        return "";
#endif
    }

    std::string ModuleLoader::sourceStdlibPath() {
#ifdef BLOCH_STDLIB_SOURCE_DIR
        return std::string(BLOCH_STDLIB_SOURCE_DIR);
#else
        return "";
#endif
    }

    void ModuleLoader::processImports(Program& program, const std::string& fromDir) {
        for (auto& imp : program.imports) {
            std::string target = resolveImportPath(imp->path, fromDir);
            if (target.empty()) {
                throw BlochError(ErrorCategory::Semantic, imp->line, imp->column,
                                 "import '" + joinQualified(imp->path) + "' not found");
            }
            loadModule(target);
        }
        program.imports.clear();
    }

    void ModuleLoader::loadModule(const std::string& path) {
        std::string canon = canonicalize(path);
        auto cyc = std::find(m_stack.begin(), m_stack.end(), canon);
        if (cyc != m_stack.end()) {
            std::ostringstream oss;
            oss << "import cycle detected: ";
            for (auto it = cyc; it != m_stack.end(); ++it) oss << *it << " -> ";
            oss << canon;
            throw BlochError(ErrorCategory::Semantic, 0, 0, oss.str());
        }
        if (m_cache.count(canon))
            return;

        m_stack.push_back(canon);
        std::unique_ptr<Program> program = parseFile(canon);

        fs::path parent = fs::path(canon).parent_path();
        processImports(*program, parent.string());
        m_cache[canon] = std::move(program);
        m_loadOrder.push_back(canon);
        m_stack.pop_back();
    }

    std::unique_ptr<Program> ModuleLoader::load(const std::string& entryFile) {
        m_cache.clear();
        m_loadOrder.clear();
        m_stack.clear();

        loadModule(entryFile);

        std::unique_ptr<Program> merged = std::make_unique<Program>();
        for (const auto& path : m_loadOrder) {
            std::unique_ptr<Program>& mod = m_cache[path];
            for (auto& cls : mod->classes) merged->classes.push_back(std::move(cls));
            for (auto& fn : mod->functions) merged->functions.push_back(std::move(fn));
            for (auto& stmt : mod->statements) merged->statements.push_back(std::move(stmt));
        }

        size_t mainCount = 0;
        for (std::unique_ptr<FunctionDeclaration>& fn : merged->functions) {
            if (fn && fn->name == "main") {
                ++mainCount;
                if (fn->hasShotsAnnotation) {
                    for (std::unique_ptr<AnnotationNode>& annotation : fn->annotations) {
                        if (annotation && annotation->name == "shots") {
                            int shotCount = std::stoi(annotation->value);
                            merged->shots = {true, shotCount};
                        }
                    }
                } else {
                    merged->shots = {false, 1};
                }
            }
        }

        if (mainCount == 0) {
            throw BlochError(ErrorCategory::Semantic, 0, 0,
                             "No 'main' function found across imported modules");
        }
        if (mainCount > 1) {
            throw BlochError(ErrorCategory::Semantic, 0, 0,
                             "Multiple 'main' functions found across imported modules");
        }

        return merged;
    }

}  // namespace bloch::compiler
