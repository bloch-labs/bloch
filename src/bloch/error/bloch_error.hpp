#pragma once

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace bloch {
    // We use simple, coloured messages to make CLI output easy to scan.
    enum class MessageLevel { Info, Warning, Error };

    inline const char* colour(MessageLevel level) {
        switch (level) {
            case MessageLevel::Info:
                // Green
                return "\033[32m";
            case MessageLevel::Warning:
                // Orange
                return "\033[38;5;208m";
            case MessageLevel::Error:
            default:
                // Red
                return "\033[31m";
        }
    }

    inline const char* prefix(MessageLevel level) {
        switch (level) {
            case MessageLevel::Info:
                return "[INFO]:";
            case MessageLevel::Warning:
                return "[WARNING]:";
            case MessageLevel::Error:
            default:
                return "[ERROR]:";
        }
    }

    // Compose a single-line message; when line/column are 0 we omit the location.
    inline std::string format(MessageLevel level, int line, int column, const std::string& msg) {
        std::ostringstream err;
        err << colour(level) << prefix(level) << " ";
        if (line > 0 && column > 0)
            err << "Line " << line << ", Col " << column << ": ";
        // Reset colour before newline so subsequent output is unaffected
        err << msg << "\033[0m\n";
        return err.str();
    }

    class BlochError : public std::runtime_error {
       public:
        BlochError(int line, int column, const std::string& msg)
            : std::runtime_error(format(MessageLevel::Error, line, column, msg)),
              line(line),
              column(column) {}

        int line;
        int column;
    };

    inline void blochInfo(int line, int column, const std::string& msg) {
        std::cerr << format(MessageLevel::Info, line, column, msg);
    }

    inline void blochWarning(int line, int column, const std::string& msg) {
        std::cerr << format(MessageLevel::Warning, line, column, msg);
    }
}
