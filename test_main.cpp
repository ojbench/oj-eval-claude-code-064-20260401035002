#include "printf.hpp"
#include <iostream>
#include <vector>

int main() {
    // Test 1: Basic string
    sjtu::printf("%s\n", "Hello World");

    // Test 2: Integer with %d
    sjtu::printf("%d\n", 42);

    // Test 3: Unsigned with %u
    sjtu::printf("%u\n", 100u);

    // Test 4: Escaped %%
    sjtu::printf("%%d\n");

    // Test 5: Default format %_
    sjtu::printf("%_\n", 123);
    sjtu::printf("%_\n", "test");

    // Test 6: Vector with default format
    std::vector<int> vec = {1, 2, 3};
    sjtu::printf("%_\n", vec);

    // Test 7: Mixed formats
    sjtu::printf("Value: %d, String: %s\n", 42, "hello");

    return 0;
}
