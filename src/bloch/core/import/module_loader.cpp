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

#include "bloch/core/import/module_loader.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "bloch/core/lexer/lexer.hpp"
#include "bloch/core/parser/parser.hpp"
#include "bloch/support/error/bloch_error.hpp"

namespace bloch::core {

    using support::BlochError;
    using support::ErrorCategory;
    namespace fs = std::filesystem;

    ModuleLoader::ModuleLoader(std::vector<std::string> searchPaths)
        : m_searchPaths(std::move(searchPaths)) {}

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
        auto tokens = lexer.tokenize();
        Parser parser(std::move(tokens));
        return parser.parse();
    }

    std::string ModuleLoader::resolveImportPath(const std::vector<std::string>& parts,
                                                const std::string& fromDir) const {
        fs::path relative;
        for (const auto& p : parts) relative /= p;
        relative += ".bloch";

        // Search order: importing file's directory, provided search paths, current working dir.
        std::vector<fs::path> bases;
        bases.push_back(fromDir);
        for (const auto& p : m_searchPaths) bases.emplace_back(p);
        bases.push_back(fs::current_path());

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
        auto program = parseFile(canon);

        fs::path parent = fs::path(canon).parent_path();
        for (auto& imp : program->imports) {
            std::string target = resolveImportPath(imp->path, parent.string());
            if (target.empty()) {
                throw BlochError(ErrorCategory::Semantic, imp->line, imp->column,
                                 "import '" + joinQualified(imp->path) + "' not found");
            }
            loadModule(target);
        }

        program->imports.clear();
        m_cache[canon] = std::move(program);
        m_loadOrder.push_back(canon);
        m_stack.pop_back();
    }

    std::unique_ptr<Program> ModuleLoader::load(const std::string& entryFile) {
        m_cache.clear();
        m_loadOrder.clear();
        m_stack.clear();

        loadModule(entryFile);

        auto merged = std::make_unique<Program>();
        for (const auto& path : m_loadOrder) {
            auto& mod = m_cache[path];
            for (auto& cls : mod->classes) merged->classes.push_back(std::move(cls));
            for (auto& fn : mod->functions) merged->functions.push_back(std::move(fn));
            for (auto& stmt : mod->statements) merged->statements.push_back(std::move(stmt));
        }

        size_t mainCount = 0;
        for (auto& fn : merged->functions)
            if (fn && fn->name == "main")
                ++mainCount;

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

}  // namespace bloch::core
