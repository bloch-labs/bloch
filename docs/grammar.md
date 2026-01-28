---
title: Bloch Grammar (EBNF)
---
# Bloch Grammar (EBNF)

This grammar reflects the v1.0.0 release candidate as implemented in the parser and validated by the semantic analyser. Comments use `// ...` and are ignored by the lexer.

```
program            = { function | statement } ;

function           = { annotation } "function" identifier
                      "(" [ parameterList ] ")" "->" type block ;

annotation         = "@" ( "quantum" | "tracked" ) ;

parameterList      = parameter { "," parameter } ;
parameter          = type identifier ;

block              = "{" { statement } "}" ;

statement          = block
                   | [ "final" ] variableDeclaration
                   | "return" [ expression ] ";"
                   | "if" "(" expression ")" block [ "else" block ]
                   | "for" "(" [ forInit ] ";" expression ";" expression ")" block
                   | "while" "(" expression ")" block
                   | "echo" "(" expression ")" ";"
                   | "reset" expression ";"
                   | "measure" expression ";"
                   | expression "?" statement ":" statement
                   | assignment
                   | expressionStatement ;

forInit            = variableDeclaration | expressionStatement ;

variableDeclaration = annotations? type identifier { "," identifier }
                      [ "=" expression ] ";" ;

annotations        = annotation { annotation } ;

assignment         = identifier "=" expression ";" ;
expressionStatement= expression ";" ;

expression         = assignmentExpression ;
assignmentExpression = logicalOr [ "=" assignmentExpression ] ;
logicalOr          = logicalAnd { "||" logicalAnd } ;
logicalAnd         = bitwiseOr { "&&" bitwiseOr } ;
bitwiseOr          = bitwiseXor { "|" bitwiseXor } ;
bitwiseXor         = bitwiseAnd { "^" bitwiseAnd } ;
bitwiseAnd         = equality { "&" equality } ;
equality           = comparison { ("==" | "!=") comparison } ;
comparison         = additive { (">" | "<" | ">=" | "<=") additive } ;
additive           = multiplicative { ("+" | "-") multiplicative } ;
multiplicative     = unary { ("*" | "/" | "%") unary } ;
unary              = ("-" | "!" | "~") unary | postfix ;
postfix            = primary { ( "(" [ argumentList ] ")" )
                               | ("[" expression "]")
                               | ("++" | "--") } ;
argumentList       = expression { "," expression } ;
primary            = literal
                   | "measure" expression
                   | identifier
                   | "(" expression ")"
                   | "{" [ expression { "," expression } ] "}" ;

literal            = integerLiteral
                   | floatLiteral
                   | bitLiteral
                   | stringLiteral
                   | charLiteral ;

type               = primitiveType [ "[" [ integerLiteral ] "]" ] ;
primitiveType      = "void" | "int" | "float" | "char" | "string"
                   | "bit" | "qubit" ;

integerLiteral     = digit { digit } ;
floatLiteral       = digit { digit } "." digit { digit } "f" ;
bitLiteral         = ("0" | "1") "b" ;
```


Notes:
- The `? :` conditional in Bloch is a statement form: `expr ? statement : statement`.
- Arrays use `{ ... }` literals; `qubit[]` cannot be initialised.
- Float literals require an `f` suffix, e.g. `3.14f`. Bit literals require `b`, e.g. `1b`.
