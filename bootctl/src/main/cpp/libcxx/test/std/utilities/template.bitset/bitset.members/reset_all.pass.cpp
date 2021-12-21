//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test bitset<N>& reset();

#include <bitset>
#include <cassert>

#include "test_macros.h"

#if defined(TEST_COMPILER_CLANG)
#pragma clang diagnostic ignored "-Wtautological-compare"
#elif defined(TEST_COMPILER_C1XX)
#pragma warning(disable: 6294) // Ill-defined for-loop:  initial condition does not satisfy test.  Loop body not executed.
#endif

template <std::size_t N>
void test_reset_all()
{
    std::bitset<N> v;
    v.set();
    v.reset();
    for (std::size_t i = 0; i < N; ++i)
        assert(!v[i]);
}

int main()
{
    test_reset_all<0>();
    test_reset_all<1>();
    test_reset_all<31>();
    test_reset_all<32>();
    test_reset_all<33>();
    test_reset_all<63>();
    test_reset_all<64>();
    test_reset_all<65>();
    test_reset_all<1000>();
}
