#pragma once

#include <string>

namespace bloch
{
    enum class TokenType
    {
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
        Logical,
        Void,
        Function,
        Import,
        Return,
        If,
        For,
        Class,
        Measure,
        Final,
        Reset,
        Public,
        Private,

        // Annotations
        At,
        Quantum,
        State,
        Adjoint,
        Members,
        Methods,

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
        Semicolon,
        Comma,
        Dot,
        Colon,
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

    struct Token
    {
        TokenType type;
        std::string value;
        int line;
        int column;
    };
}
