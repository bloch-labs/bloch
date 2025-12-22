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

#include <string>

namespace bloch::core {
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
        Default,

        // Annotations
        At,
        Quantum,
        Tracked,

        // Class System
        Class,
        Public,
        Private,
        Protected,
        Static,
        Extends,
        Abstract,
        Virtual,
        Override,
        Super,
        This,
        Import,
        New,
        Constructor,
        Destructor,
        Destroy,

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
        Dot,
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
}  // namespace bloch::core
