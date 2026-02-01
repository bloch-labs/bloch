# Bloch Grammar (EBNF)

This grammar reflects the current parser. Some constraints (such as valid type casts, `@shots`
placement, or `super(...)` rules) are enforced semantically; see `language-spec.md`.

```
program          = { importDecl | classDecl | functionDecl | statement } ;

importDecl       = "import" qualifiedName ";" ;
qualifiedName    = identifier { "." identifier } ;

classDecl        = { "static" | "abstract" } "class" identifier
                   [ typeParams ] [ "extends" type ] "{" { classMember } "}" ;

typeParams       = "<" [ typeParam { "," typeParam } ] ">" ;
typeParam        = identifier [ "extends" type ] ;

classMember      = { classModifier } ( constructorDecl | destructorDecl
                                      | methodDecl | fieldDecl ) ;
classModifier    = visibility | "static" | "virtual" | "override" ;
visibility       = "public" | "private" | "protected" ;

constructorDecl  = "constructor" "(" [ parameterList ] ")" "->" identifier
                   ( block | "=" "default" ";" ) ;
destructorDecl   = "destructor" "(" ")" "->" "void"
                   ( block | "=" "default" ";" ) ;
methodDecl       = { funcAnnotation } "function" identifier
                   "(" [ parameterList ] ")" "->" type ( block | ";" ) ;
fieldDecl        = [ "final" ] [ varAnnotation ] type identifier
                   [ "=" expression ] ";" ;

functionDecl     = { funcAnnotation } "function" identifier
                   "(" [ parameterList ] ")" "->" type block ;

varAnnotation    = "@" "tracked" ;
funcAnnotation   = "@" "quantum" | "@" "shots" "(" integerLiteral ")" ;

parameterList    = parameter { "," parameter } ;
parameter        = type identifier ;

block            = "{" { statement } "}" ;

statement        = block
                 | variableDecl
                 | returnStmt
                 | ifStmt
                 | forStmt
                 | whileStmt
                 | echoStmt
                 | resetStmt
                 | measureStmt
                 | destroyStmt
                 | ternaryStmt
                 | assignmentStmt
                 | expressionStmt ;

variableDecl     = [ "final" ] [ varAnnotation ] type identifier
                   { "," identifier } [ "=" expression ] ";" ;
assignmentStmt   = identifier "=" expression ";" ;
expressionStmt   = expression ";" ;
returnStmt       = "return" [ expression ] ";" ;
ifStmt           = "if" "(" expression ")" block [ "else" block ] ;
forStmt          = "for" "(" [ forInit ] ";" expression ";" expression ")" block ;
forInit          = variableDecl | expressionStmt ;
whileStmt        = "while" "(" expression ")" block ;
echoStmt         = "echo" "(" expression ")" ";" ;
resetStmt        = "reset" expression ";" ;
measureStmt      = "measure" expression ";" ;
destroyStmt      = "destroy" expression ";" ;
ternaryStmt      = expression "?" statement ":" statement ;

expression       = assignmentExpression ;
assignmentExpression = logicalOr [ "=" assignmentExpression ] ;
logicalOr        = logicalAnd { "||" logicalAnd } ;
logicalAnd       = bitwiseOr { "&&" bitwiseOr } ;
bitwiseOr        = bitwiseXor { "|" bitwiseXor } ;
bitwiseXor       = bitwiseAnd { "^" bitwiseAnd } ;
bitwiseAnd       = equality { "&" equality } ;
equality         = comparison { ("==" | "!=") comparison } ;
comparison       = additive { (">" | "<" | ">=" | "<=") additive } ;
additive         = multiplicative { ("+" | "-") multiplicative } ;
multiplicative   = unary { ("*" | "/" | "%") unary } ;
unary            = ("-" | "!" | "~") unary | postfix ;
postfix          = primary { ( "(" [ argumentList ] ")" )
                           | ("[" expression "]")
                           | ("." identifier)
                           | ("++" | "--") } ;
argumentList     = expression { "," expression } ;

primary          = literal
                 | "null"
                 | "measure" expression
                 | "this"
                 | "super"
                 | "new" type "(" [ argumentList ] ")"
                 | identifier
                 | "(" expression ")"
                 | "(" type ")" unary
                 | "{" [ expression { "," expression } ] "}" ;

literal          = integerLiteral
                 | longLiteral
                 | floatLiteral
                 | bitLiteral
                 | stringLiteral
                 | charLiteral
                 | booleanLiteral ;

booleanLiteral   = "true" | "false" ;

// Types

type             = baseType { "[" [ integerLiteral | expression ] "]" } ;
baseType         = primitiveType | namedType ;
primitiveType    = "void" | "int" | "long" | "float" | "char" | "string"
                 | "bit" | "boolean" | "qubit" ;
namedType        = qualifiedName [ typeArguments ] ;
typeArguments    = "<" type { "," type } ">" ;

integerLiteral   = digit { digit } ;
longLiteral      = digit { digit } "L" ;
floatLiteral     = digit { digit } [ "." digit { digit } ] "f" ;
bitLiteral       = ("0" | "1") "b" ;
stringLiteral    = '"' { any-char } '"' ;
charLiteral      = "'" any-char "'" ;
```
