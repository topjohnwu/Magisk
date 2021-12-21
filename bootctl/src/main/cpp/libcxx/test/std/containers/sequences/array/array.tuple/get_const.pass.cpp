//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <array>

// template <size_t I, class T, size_t N> const T& get(const array<T, N>& a);

#include <array>
#include <cassert>

#include "test_macros.h"

// std::array is explicitly allowed to be initialized with A a = { init-list };.
// Disable the missing braces warning for this reason.
#include "disable_missing_braces_warning.h"

int main()
{
    {
        typedef double T;
        typedef std::array<T, 3> C;
        const C c = {1, 2, 3.5};
        assert(std::get<0>(c) == 1);
        assert(std::get<1>(c) == 2);
        assert(std::get<2>(c) == 3.5);
    }
#if TEST_STD_VER > 11
    {
        typedef double T;
        typedef std::array<T, 3> C;
        constexpr const C c = {1, 2, 3.5};
        static_assert(std::get<0>(c) == 1, "");
        static_assert(std::get<1>(c) == 2, "");
        static_assert(std::get<2>(c) == 3.5, "");
    }
#endif
}
