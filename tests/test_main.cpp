#include "test_harness.hpp"

#include <exception>
#include <iostream>

int main() {
    std::size_t failures = 0;
    for (const auto& test : hltest::registry()) {
        try {
            test.function();
            std::cout << "[PASS] " << test.name << '\n';
        } catch (const std::exception& error) {
            ++failures;
            std::cerr << "[FAIL] " << test.name << ": " << error.what() << '\n';
        } catch (...) {
            ++failures;
            std::cerr << "[FAIL] " << test.name << ": unknown exception\n";
        }
    }

    std::cout << "Executed " << hltest::registry().size() << " tests; "
              << failures << " failed.\n";
    return failures == 0 ? 0 : 1;
}
