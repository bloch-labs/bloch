#pragma once

#include <string>

namespace bloch {
    enum class TokenType {
        // Literals
        Identifier,
        IntegerLiteral,
        FloatLiteral,
        StringLiteral,
        CharLiteral,

        // Keywords
        Int,
        Float,
        String,
        Char,
        Qubit,
        Bit,
        Void,
        Function,
        Import,
        Return,
        If,
        Else,
        For,
        While,
        Measure,
        Final,
        Reset,

        // Annotations
        At,
        Quantum,
        Adjoint,

        // Operators and Punctuation
        Equals,
        Plus,
        Minus,
        Star,
        Slash,
        Percent,
        Greater,
        GreaterEqual,
        Less,
        LessEqual,
        EqualEqual,
        Bang,
        BangEqual,
        Semicolon,
        Comma,
        Arrow,
        LParen,
        RParen,
        LBrace,
        RBrace,
        LBracket,
        RBracket,

        // Built-ins
        Echo,

        // Control
        Eof,
        Unknown
    };

    struct Token {
        TokenType type;
        std::string value;
        int line;
        int column;
    };
}
