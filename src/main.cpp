#include <iostream>
#include "bloch/lexer/lexer.hpp"
#include "bloch/version/version.hpp"

int main() {
    std::cout << "Bloch Language Compiler v" << bloch::version << std::endl;

    bloch::Lexer lexer("");

    return 0;
}