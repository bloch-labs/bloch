#include <iostream>
#include "bloch/version/version.hpp"

int main() {
    std::cout << "Bloch Language Compiler v" << bloch::version << std::endl;
    return 0;
}