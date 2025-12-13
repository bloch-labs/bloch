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

#include "bloch/core/semantics/built_ins.hpp"

namespace bloch::core {

    const std::unordered_map<std::string, BuiltInGate> builtInGates = {
        {"h", BuiltInGate{"h", {ValueType::Qubit}, ValueType::Void}},
        {"x", BuiltInGate{"x", {ValueType::Qubit}, ValueType::Void}},
        {"y", BuiltInGate{"y", {ValueType::Qubit}, ValueType::Void}},
        {"z", BuiltInGate{"z", {ValueType::Qubit}, ValueType::Void}},
        {"rx", BuiltInGate{"rx", {ValueType::Qubit, ValueType::Float}, ValueType::Void}},
        {"ry", BuiltInGate{"ry", {ValueType::Qubit, ValueType::Float}, ValueType::Void}},
        {"rz", BuiltInGate{"rz", {ValueType::Qubit, ValueType::Float}, ValueType::Void}},
        {"cx", BuiltInGate{"cx", {ValueType::Qubit, ValueType::Qubit}, ValueType::Void}},
    };

}  // namespace bloch::core
