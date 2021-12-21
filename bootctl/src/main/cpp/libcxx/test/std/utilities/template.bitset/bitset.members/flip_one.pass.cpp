//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test bitset<N>& flip(size_t pos);

#include <bitset>
#include <cstdlib>
#include <cassert>
#include <stdexcept>

#include "test_macros.h"

#if defined(TEST_COMPILER_C1XX)
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
void test_flip_one(bool test_throws)
{
    std::bitset<N> v = make_bitset<N>();
#ifdef TEST_HAS_NO_EXCEPTIONS
    if (test_throws) return;
#else
    try
    {
#endif
        v.flip(50);
        bool b = v[50];
        if (50 >= v.size())
            assert(false);
        assert(v[50] == b);
        v.flip(50);
        assert(v[50] != b);
        v.flip(50);
        assert(v[50] == b);
        assert(!test_throws);
#ifndef TEST_HAS_NO_EXCEPTIONS
    }
    catch (std::out_of_range&)
    {
        assert(test_throws);
    }
#endif
}

int main()
{
    test_flip_one<0>(true);
    test_flip_one<1>(true);
    test_flip_one<31>(true);
    test_flip_one<32>(true);
    test_flip_one<33>(true);
    test_flip_one<63>(false);
    test_flip_one<64>(false);
    test_flip_one<65>(false);
    test_flip_one<1000>(false);
}
