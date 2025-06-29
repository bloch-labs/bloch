#pragma once

#include <sstream>
#include <stdexcept>
#include <string>

namespace bloch {
    class BlochRuntimeError : public std::runtime_error {
       public:
        BlochRuntimeError(const std::string& prefix, int line, int column, const std::string& msg)
            : std::runtime_error(format(prefix, line, column, msg)), line(line), column(column) {}

        int line;
        int column;

       private:
        static std::string format(const std::string& prefix, int line, int column,
                                  const std::string& msg) {
            std::ostringstream err;
            err << "[" << prefix << "]\n"
                << "Line " << line << ", Col " << column << ": " << msg << "\n";
            return err.str();
        }
    };
}