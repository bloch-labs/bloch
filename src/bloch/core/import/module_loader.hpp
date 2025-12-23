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

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "bloch/core/ast/ast.hpp"

namespace bloch::core {

    // Resolves and loads imports starting from an entry file, producing a single
    // aggregated Program ready for semantic analysis and execution. Imports are
    // resolved relative to the importing file and optional search paths.
    class ModuleLoader {
       public:
        explicit ModuleLoader(std::vector<std::string> searchPaths = {});

        // Load the entry file and all of its transitive imports, merging them
        // into a single Program. Throws BlochError on missing modules, cycles,
        // or parse errors. Exactly one main() must exist across the graph.
        std::unique_ptr<Program> load(const std::string& entryFile);

       private:
        std::vector<std::string> m_searchPaths;
        std::unordered_map<std::string, std::unique_ptr<Program>> m_cache;
        std::vector<std::string> m_loadOrder;
        std::vector<std::string> m_stack;

        std::unique_ptr<Program> parseFile(const std::string& path) const;
        void loadModule(const std::string& path);
        std::string resolveImportPath(const std::vector<std::string>& parts,
                                      const std::string& fromDir) const;
        std::string canonicalize(const std::string& path) const;
        static std::string joinQualified(const std::vector<std::string>& parts);
    };

}  // namespace bloch::core
