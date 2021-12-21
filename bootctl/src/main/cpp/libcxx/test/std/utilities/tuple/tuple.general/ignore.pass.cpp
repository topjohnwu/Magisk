//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <tuple>

// constexpr unspecified ignore;

// UNSUPPORTED: c++98, c++03

#include <tuple>
#include <cassert>

#include "test_macros.h"

constexpr bool test_ignore_constexpr()
{
#if TEST_STD_VER > 11
    { // Test that std::ignore provides constexpr converting assignment.
        auto& res = (std::ignore = 42);
        assert(&res == &std::ignore);
    }
    { // Test that std::ignore provides constexpr copy/move constructors
        auto copy = std::ignore;
        auto moved = std::move(copy);
        ((void)moved);
    }
    { // Test that std::ignore provides constexpr copy/move assignment
        auto copy = std::ignore;
        copy = std::ignore;
        auto moved = std::ignore;
        moved = std::move(copy);
    }
#endif
    return true;
}

int main() {
    {
        constexpr auto& ignore_v = std::ignore;
        ((void)ignore_v);
    }
    {
        static_assert(test_ignore_constexpr(), "");
    }
    {
        LIBCPP_STATIC_ASSERT(std::is_trivial<decltype(std::ignore)>::value, "");
    }
}
