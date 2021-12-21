//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <array>

// template <size_t I, class T, size_t N> T& get(array<T, N>& a);

#include <array>
#include <cassert>

#include "test_macros.h"

// std::array is explicitly allowed to be initialized with A a = { init-list };.
// Disable the missing braces warning for this reason.
#include "disable_missing_braces_warning.h"


#if TEST_STD_VER > 11
struct S {
   std::array<int, 3> a;
   int k;
   constexpr S() : a{1,2,3}, k(std::get<2>(a)) {}
};

constexpr std::array<int, 2> getArr () { return { 3, 4 }; }
#endif

int main()
{
    {
        typedef double T;
        typedef std::array<T, 3> C;
        C c = {1, 2, 3.5};
        std::get<1>(c) = 5.5;
        assert(c[0] == 1);
        assert(c[1] == 5.5);
        assert(c[2] == 3.5);
    }
#if TEST_STD_VER > 11
    {
        typedef double T;
        typedef std::array<T, 3> C;
        constexpr C c = {1, 2, 3.5};
        static_assert(std::get<0>(c) == 1, "");
        static_assert(std::get<1>(c) == 2, "");
        static_assert(std::get<2>(c) == 3.5, "");
    }
    {
        static_assert(S().k == 3, "");
        static_assert(std::get<1>(getArr()) == 4, "");
    }
#endif
}
