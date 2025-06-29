#include <iostream>
#include "bloch/version/version.hpp"
#include "bloch/lexer/lexer.hpp"

int main() {
    std::cout << "Bloch Language Compiler v" << bloch::version << std::endl;

    bloch::Lexer lexer();
    
    return 0;
}