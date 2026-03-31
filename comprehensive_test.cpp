#include "printf.hpp"
#include <iostream>
#include <vector>
#include <cassert>
#include <sstream>

// Helper to capture cout
std::stringstream captured_output;
std::streambuf* old_cout;

void start_capture() {
    old_cout = std::cout.rdbuf(captured_output.rdbuf());
}

std::string end_capture() {
    std::cout.rdbuf(old_cout);
    std::string result = captured_output.str();
    captured_output.str("");
    captured_output.clear();
    return result;
}

int main() {
    // Test 1: Basic string
    start_capture();
    sjtu::printf("%s", "Hello");
    assert(end_capture() == "Hello");
    std::cout << "Test 1 passed: Basic string\n";

    // Test 2: Integer with %d
    start_capture();
    sjtu::printf("%d", 42);
    assert(end_capture() == "42");
    std::cout << "Test 2 passed: Integer %d\n";

    // Test 3: Unsigned with %u
    start_capture();
    sjtu::printf("%u", 100u);
    assert(end_capture() == "100");
    std::cout << "Test 3 passed: Unsigned %u\n";

    // Test 4: Escaped %%
    start_capture();
    sjtu::printf("%%d");
    assert(end_capture() == "%d");
    std::cout << "Test 4 passed: Escaped %%\n";

    // Test 5: Default format %_ for int
    start_capture();
    sjtu::printf("%_", 123);
    assert(end_capture() == "123");
    std::cout << "Test 5 passed: Default %_ for int\n";

    // Test 6: Default format %_ for string
    start_capture();
    sjtu::printf("%_", "test");
    assert(end_capture() == "test");
    std::cout << "Test 6 passed: Default %_ for string\n";

    // Test 7: Vector with default format
    std::vector<int> vec = {1, 2, 3};
    start_capture();
    sjtu::printf("%_", vec);
    assert(end_capture() == "[1,2,3]");
    std::cout << "Test 7 passed: Vector %_\n";

    // Test 8: Mixed formats
    start_capture();
    sjtu::printf("Value: %d, String: %s", 42, "hello");
    assert(end_capture() == "Value: 42, String: hello");
    std::cout << "Test 8 passed: Mixed formats\n";

    // Test 9: Multiple %% escapes
    start_capture();
    sjtu::printf("%%s %%d %%u");
    assert(end_capture() == "%s %d %u");
    std::cout << "Test 9 passed: Multiple escapes\n";

    // Test 10: Signed to unsigned conversion
    start_capture();
    sjtu::printf("%u", -1);
    assert(end_capture() == "18446744073709551615"); // uint64_t max
    std::cout << "Test 10 passed: Signed to unsigned\n";

    std::cout << "\nAll tests passed!\n";
    return 0;
}
