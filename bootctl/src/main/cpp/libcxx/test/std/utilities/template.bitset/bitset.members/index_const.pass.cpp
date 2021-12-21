//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test constexpr bool operator[](size_t pos) const;

#include <bitset>
#include <type_traits>
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
void test_index_const()
{
    const std::bitset<N> v1 = make_bitset<N>();
    const bool greater_than_0 = std::integral_constant<bool, (N > 0)>::value; // avoid compiler warnings
    if (greater_than_0)
    {
        assert(v1[N/2] == v1.test(N/2));
    }
}

int main()
{
    test_index_const<0>();
    test_index_const<1>();
    test_index_const<31>();
    test_index_const<32>();
    test_index_const<33>();
    test_index_const<63>();
    test_index_const<64>();
    test_index_const<65>();
    test_index_const<1000>();
}
