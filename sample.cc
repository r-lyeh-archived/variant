#include <iostream>
//#include "petitsuite/petitsuite.hpp"
#include "variant.hpp"

int main() {

    variant<double> var;

    var = 10;               // -> 10
    std::cout << var << std::endl;

    var = "hello";          // -> "hello"
    std::cout << var << std::endl;

    var += 10;              // -> "hello10"
    std::cout << var << std::endl;

    var = "10";             // -> "10"
    var += 10;              // -> "1010"
    std::cout << var << std::endl;

    var = "10";             // -> "10"
    var = var.flat();       // -> 10
    var += 10;              // -> 20
    std::cout << var << std::endl;

    std::cout << "sizeof(var) = " << sizeof(var) << std::endl;
    std::cout << var.tests() << " tests done." << std::endl;
    std::cout << "All ok." << std::endl;

    return 0;
}
