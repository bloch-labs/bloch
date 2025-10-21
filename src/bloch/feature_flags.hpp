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

#include <array>
#include <cstddef>
#include <span>
#include <string_view>
#include <vector>

// ---------------------------------------------------------------------------
//  Compilation defaults
// ---------------------------------------------------------------------------
//  CMake only defines BLOCH_FEATURE_* macros for enabled flags. We provide
//  zero-valued fallbacks so the rest of the header can treat every flag as a
//  well-formed constant expression.
// ---------------------------------------------------------------------------

#ifndef BLOCH_FEATURE_BLOCH_CLASS_SYSTEM
#define BLOCH_FEATURE_BLOCH_CLASS_SYSTEM 0
#endif

namespace bloch::feature_flags {

    struct FeatureFlag {
        std::string_view name;
        bool enabled;
    };

    inline constexpr std::string_view BLOCH_CLASS_SYSTEM = "BLOCH_CLASS_SYSTEM";

    namespace detail {
        inline constexpr auto registry =
            std::array{FeatureFlag{BLOCH_CLASS_SYSTEM, BLOCH_FEATURE_BLOCH_CLASS_SYSTEM}};

        inline constexpr auto enabled_count = [] {
            std::size_t count = 0;
            for (const auto& entry : registry)
                if (entry.enabled)
                    ++count;
            return count;
        }();
    }  // namespace detail

    inline constexpr auto all_flags = [] {
        std::array<std::string_view, detail::registry.size()> names{};
        for (std::size_t idx = 0; idx < detail::registry.size(); ++idx)
            names[idx] = detail::registry[idx].name;
        return names;
    }();

    [[nodiscard]] inline constexpr std::span<const FeatureFlag> defined_flags() {
        return detail::registry;
    }

    [[nodiscard]] inline constexpr std::size_t flag_count() { return detail::registry.size(); }

    [[nodiscard]] inline constexpr std::size_t enabled_flag_count() {
        return detail::enabled_count;
    }

    [[nodiscard]] inline constexpr bool is_known(std::string_view flag) {
        for (auto candidate : all_flags) {
            if (candidate == flag)
                return true;
        }
        return false;
    }

    [[nodiscard]] inline constexpr bool is_enabled(std::string_view flag) {
        for (const auto& entry : detail::registry) {
            if (entry.name == flag)
                return entry.enabled;
        }
        return false;
    }

    [[nodiscard]] inline constexpr bool any_enabled() { return detail::enabled_count > 0; }

    [[nodiscard]] inline std::vector<std::string_view> enabled_flags() {
        std::vector<std::string_view> result;
        result.reserve(detail::enabled_count);
        for (const auto& entry : detail::registry) {
            if (entry.enabled)
                result.push_back(entry.name);
        }
        return result;
    }

    [[nodiscard]] inline std::vector<std::string_view> disabled_flags() {
        std::vector<std::string_view> result;
        result.reserve(detail::registry.size() - detail::enabled_count);
        for (const auto& entry : detail::registry) {
            if (!entry.enabled)
                result.push_back(entry.name);
        }
        return result;
    }

}  // namespace bloch::feature_flags

#ifdef BLOCH_FEATURE_ENABLED
#undef BLOCH_FEATURE_ENABLED
#endif
#define BLOCH_FEATURE_ENABLED(name) (BLOCH_FEATURE_##name != 0)

#ifdef BLOCH_FEATURE_IS_ENABLED
#undef BLOCH_FEATURE_IS_ENABLED
#endif
#define BLOCH_FEATURE_IS_ENABLED(flag_literal) (::bloch::feature_flags::is_enabled(flag_literal))
