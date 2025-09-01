#pragma once

#include <string>

namespace bloch {
    // Tokens represent the smallest meaningful pieces the parser understands.
    // We group them loosely by purpose to keep scanning and parsing readable.
    enum class TokenType {
        // Literals
        Identifier,
        IntegerLiteral,
        FloatLiteral,
        BitLiteral,
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
        Tracked,

        // Operators and Punctuation
        Equals,
        Plus,
        PlusPlus,
        Minus,
        MinusMinus,
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
        Ampersand,
        AmpersandAmpersand,
        Pipe,
        PipePipe,
        Caret,
        Tilde,
        Question,
        Colon,
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

    // A token carries what it is, the raw text we saw, and where it came from.
    // Line/column are 1-based and point to the start of the token for friendly errors.
    struct Token {
        TokenType type;
        std::string value;
        int line;
        int column;
    };
}
