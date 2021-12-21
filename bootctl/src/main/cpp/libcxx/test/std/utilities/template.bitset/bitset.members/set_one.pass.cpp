//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test bitset<N>& set(size_t pos, bool val = true);

#include <bitset>
#include <cassert>
#include <stdexcept>

#include "test_macros.h"

template <std::size_t N>
void test_set_one(bool test_throws)
{
    std::bitset<N> v;
#ifdef TEST_HAS_NO_EXCEPTIONS
    if (test_throws) return;
#else
    try
#endif
    {
        v.set(50);
        if (50 >= v.size())
            assert(false);
        assert(v[50]);
        assert(!test_throws);
    }
#ifndef TEST_HAS_NO_EXCEPTIONS
    catch (std::out_of_range&)
    {
        assert(test_throws);
    }
    try
#endif
    {
        v.set(50, false);
        if (50 >= v.size())
            assert(false);
        assert(!v[50]);
        assert(!test_throws);
    }
#ifndef TEST_HAS_NO_EXCEPTIONS
    catch (std::out_of_range&)
    {
        assert(test_throws);
    }
#endif
}

int main()
{
    test_set_one<0>(true);
    test_set_one<1>(true);
    test_set_one<31>(true);
    test_set_one<32>(true);
    test_set_one<33>(true);
    test_set_one<63>(false);
    test_set_one<64>(false);
    test_set_one<65>(false);
    test_set_one<1000>(false);
}
