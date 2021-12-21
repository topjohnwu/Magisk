//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test default ctor

#include <bitset>
#include <cassert>

#include "test_macros.h"

#if defined(TEST_COMPILER_C1XX)
#pragma warning(disable: 6294) // Ill-defined for-loop:  initial condition does not satisfy test.  Loop body not executed.
#endif

template <std::size_t N>
void test_default_ctor()
{
    {
        TEST_CONSTEXPR std::bitset<N> v1;
        assert(v1.size() == N);
        for (std::size_t i = 0; i < N; ++i)
            assert(v1[i] == false);
    }
#if TEST_STD_VER >= 11
    {
        constexpr std::bitset<N> v1;
        static_assert(v1.size() == N, "");
    }
#endif
}


int main()
{
    test_default_ctor<0>();
    test_default_ctor<1>();
    test_default_ctor<31>();
    test_default_ctor<32>();
    test_default_ctor<33>();
    test_default_ctor<63>();
    test_default_ctor<64>();
    test_default_ctor<65>();
    test_default_ctor<1000>();
}
