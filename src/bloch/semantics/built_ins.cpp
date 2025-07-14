#include "built_ins.hpp"

namespace bloch {

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

}