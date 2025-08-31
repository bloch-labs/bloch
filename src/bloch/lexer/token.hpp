#pragma once

#include <string>

namespace bloch {
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

    struct Token {
        TokenType type;
        std::string value;
        int line;
        int column;
    };
}
