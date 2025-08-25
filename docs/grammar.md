# Bloch Grammar (EBNF)

```
program        = { import | function | statement } ;

import         = "import" identifier ";" ;

function       = { annotation } "function" identifier
                  "(" [ parameterList ] ")" "->" type block ;

annotation     = "@" ( "quantum" | "adjoint" ) ;

parameterList  = parameter { "," parameter } ;
parameter      = type identifier ;

block          = "{" { statement } "}" ;

statement      = block
               | [ "final" ] variableDeclaration
               | "return" [ expression ] ";"
               | "if" "(" expression ")" block [ "else" block ]
               | "for" "(" [ forInit ] expression ";" expression ")" block
               | "while" "(" expression ")" block
               | "echo" "(" expression ")" ";"
               | "reset" expression ";"
               | "measure" expression ";"
               | expression "?" statement ";" ":" statement ";"
               | assignment
               | expressionStatement ;

forInit        = ( "final" )? variableDeclaration | expressionStatement ;

variableDeclaration = annotations? type identifier { "," identifier } [ "=" expression ] ";" ;

annotations    = annotation { annotation } ;

assignment     = identifier "=" expression ";" ;
expressionStatement = expression ";" ;

expression            = assignmentExpression ;
assignmentExpression  = comparison [ "=" assignmentExpression ] ;
comparison            = additive { (">" | "<" | ">=" | "<=") additive } ;
additive              = multiplicative { ("+" | "-") multiplicative } ;
multiplicative        = unary { ("*" | "/" | "%") unary } ;
unary                 = "-" unary | call ;
call                  = primary { "(" [ argumentList ] ")" } | "++" | "--" ;
argumentList          = expression { "," expression } ;
primary               = literal
                      | "measure" expression
                      | identifier
                      | "(" expression ")" ;

literal               = integerLiteral
                      | floatLiteral
                      | bitLiteral
                      | stringLiteral
                      | charLiteral ;

type                  = primitiveType [ "[" "]" ] ;
primitiveType         = "void" | "int" | "float" | "char" | "string"
                      | "bit" | "qubit" ;

integerLiteral        = digit { digit } ;
floatLiteral          = digit { digit } "." digit { digit } "f" ;
bitLiteral            = ("0" | "1") "b" ;
```