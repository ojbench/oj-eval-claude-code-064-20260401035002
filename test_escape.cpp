#include "printf.hpp"
#include <iostream>
int main() {
    std::cout << "Test 1: ";
    sjtu::printf("%%d");
    std::cout << "\n";

    std::cout << "Test 2: ";
    sjtu::printf("test %%d test");
    std::cout << "\n";

    std::cout << "Test 3: ";
    sjtu::printf("%%s %%d %%u");
    std::cout << "\n";

    return 0;
}
