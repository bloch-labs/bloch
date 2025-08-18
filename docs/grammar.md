# Bloch Grammar (EBNF)

```
program        = { import | function | class | statement } ;

import         = "import" identifier ";" ;

class          = "class" identifier "{" { classSection } "}" ;
classSection   = membersSection | methodsSection ;
membersSection = "@members" "(" ("\"public\"" | "\"private\"") ")" ":" { variableDeclaration } ;
methodsSection = "@methods" ":" { function } ;

function       = { annotation } "function" [ "*" ] identifier
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
               | "echo" "(" expression ")" ";"
               | "reset" expression ";"
               | "measure" expression ";"
               | assignment
               | expressionStatement ;

forInit        = ( "final" )? variableDeclaration | expressionStatement ;

variableDeclaration = annotations? type identifier [ "=" expression ] ";" ;
annotations    = annotation { annotation } ;

assignment     = identifier "=" expression ";" ;
expressionStatement = expression ";" ;

expression            = assignmentExpression ;
assignmentExpression  = comparison [ "=" assignmentExpression ] ;
comparison            = additive { (">" | "<" | ">=" | "<=") additive } ;
additive              = multiplicative { ("+" | "-") multiplicative } ;
multiplicative        = unary { ("*" | "/" | "%") unary } ;
unary                 = constructorCall | "-" unary | call ;
constructorCall       = "*" identifier "(" [ argumentList ] ")" ;
call                  = primary { ("." identifier) | "(" [ argumentList ] ")" } ;
argumentList          = expression { "," expression } ;
primary               = literal
                      | "measure" expression
                      | identifier
                      | "(" expression ")" ;

literal               = integerLiteral
                      | floatLiteral
                      | stringLiteral
                      | charLiteral ;

type                  = primitiveType [ "[" "]" ]
                      | identifier ;
primitiveType         = "void" | "int" | "float" | "char" | "string"
                      | "bit" | "qubit" ;
```