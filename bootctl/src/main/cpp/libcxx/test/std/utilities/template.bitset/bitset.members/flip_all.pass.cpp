//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test bitset<N>& flip();

#include <bitset>
#include <cstdlib>
#include <cassert>

#include "test_macros.h"

#if defined(TEST_COMPILER_CLANG)
#pragma clang diagnostic ignored "-Wtautological-compare"
#elif defined(TEST_COMPILER_C1XX)
#pragma warning(disable: 6294) // Ill-defined for-loop:  initial condition does not satisfy test.  Loop body not executed.
#endif

template <std::size_t N>
std::bitset<N>
make_bitset()
{
    std::bitset<N> v;
    for (std::size_t i = 0; i < N; ++i)
        v[i] = static_cast<bool>(std::rand() & 1);
    return v;
}

template <std::size_t N>
void test_flip_all()
{
    std::bitset<N> v1 = make_bitset<N>();
    std::bitset<N> v2 = v1;
    v2.flip();
    for (std::size_t i = 0; i < N; ++i)
        assert(v2[i] == ~v1[i]);
}

int main()
{
    test_flip_all<0>();
    test_flip_all<1>();
    test_flip_all<31>();
    test_flip_all<32>();
    test_flip_all<33>();
    test_flip_all<63>();
    test_flip_all<64>();
    test_flip_all<65>();
    test_flip_all<1000>();
}
