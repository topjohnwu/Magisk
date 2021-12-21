//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test unsigned long to_ulong() const;

#include <bitset>
#include <algorithm>
#include <type_traits>
#include <limits>
#include <climits>
#include <cassert>

template <std::size_t N>
void test_to_ulong()
{
    const std::size_t M = sizeof(unsigned long) * CHAR_BIT < N ? sizeof(unsigned long) * CHAR_BIT : N;
    const bool is_M_zero = std::integral_constant<bool, M == 0>::value; // avoid compiler warnings
    const std::size_t X = is_M_zero ? sizeof(unsigned long) * CHAR_BIT - 1 : sizeof(unsigned long) * CHAR_BIT - M;
    const std::size_t max = is_M_zero ? 0 : std::size_t(std::numeric_limits<unsigned long>::max()) >> X;
    std::size_t tests[] = {0,
                           std::min<std::size_t>(1, max),
                           std::min<std::size_t>(2, max),
                           std::min<std::size_t>(3, max),
                           std::min(max, max-3),
                           std::min(max, max-2),
                           std::min(max, max-1),
                           max};
    for (std::size_t i = 0; i < sizeof(tests)/sizeof(tests[0]); ++i)
    {
        std::size_t j = tests[i];
        std::bitset<N> v(j);
        assert(j == v.to_ulong());
    }

    { // test values bigger than can fit into the bitset
    const unsigned long val = 0x5AFFFFA5UL;
    const bool canFit = N < sizeof(unsigned long) * CHAR_BIT;
    const unsigned long mask = canFit ? (1UL << (canFit ? N : 0)) - 1 : (unsigned long)(-1); // avoid compiler warnings
    std::bitset<N> v(val);
    assert(v.to_ulong() == (val & mask)); // we shouldn't return bit patterns from outside the limits of the bitset.
    }
}

int main()
{
    test_to_ulong<0>();
    test_to_ulong<1>();
    test_to_ulong<31>();
    test_to_ulong<32>();
    test_to_ulong<33>();
    test_to_ulong<63>();
    test_to_ulong<64>();
    test_to_ulong<65>();
    test_to_ulong<1000>();
}
