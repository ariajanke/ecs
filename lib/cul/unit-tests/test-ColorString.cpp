#define MACRO_ARIAJANKE_CUL_COLORSTRING_STATIC_ASSERT_TESTS

#include <common/ColorString.hpp>

#include <iostream>

int main() {
    std::cout
        << "[ColorString] This program compiling successfully, is indication "
           "that all unit tests have passed.\n"
           "That's the power of constexpr + static_assert!" << std::endl;
    return 0;
}
