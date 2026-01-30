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

#pragma once

#ifndef BLOCH_VERSION
#define BLOCH_VERSION "dev"
#endif
#ifndef BLOCH_COMMIT_HASH
#define BLOCH_COMMIT_HASH "unknown"
#endif

#include <string_view>

namespace bloch::cli {

    struct Context {
        std::string_view version = BLOCH_VERSION;
        std::string_view commit = BLOCH_COMMIT_HASH;
    };

    int run(int argc, char** argv, const Context& ctx = {});

}  // namespace bloch::cli
