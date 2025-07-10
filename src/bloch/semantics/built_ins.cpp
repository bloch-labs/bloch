#include "built_ins.hpp"

namespace bloch {

    const std::unordered_map<std::string, BuiltInGate> builtInGates = {
        {"h", BuiltInGate{"h", {"qubit"}, "void"}},
        {"x", BuiltInGate{"x", {"qubit"}, "void"}},
        {"y", BuiltInGate{"y", {"qubit"}, "void"}},
        {"z", BuiltInGate{"z", {"qubit"}, "void"}},
        {"rx", BuiltInGate{"rx", {"qubit", "float"}, "void"}},
        {"ry", BuiltInGate{"ry", {"qubit", "float"}, "void"}},
        {"rz", BuiltInGate{"rz", {"qubit", "float"}, "void"}},
        {"cx", BuiltInGate{"cx", {"qubit", "qubit"}, "void"}},
    };

}