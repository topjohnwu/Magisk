//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test unsigned long long to_ullong() const;

#include <bitset>
#include <algorithm>
#include <type_traits>
#include <climits>
#include <cassert>

template <std::size_t N>
void test_to_ullong()
{
    const std::size_t M = sizeof(unsigned long long) * CHAR_BIT < N ? sizeof(unsigned long long) * CHAR_BIT : N;
    const bool is_M_zero = std::integral_constant<bool, M == 0>::value; // avoid compiler warnings
    const std::size_t X = is_M_zero ? sizeof(unsigned long long) * CHAR_BIT - 1 : sizeof(unsigned long long) * CHAR_BIT - M;
    const unsigned long long max = is_M_zero ? 0 : (unsigned long long)(-1) >> X;
    unsigned long long tests[] = {0,
                           std::min<unsigned long long>(1, max),
                           std::min<unsigned long long>(2, max),
                           std::min<unsigned long long>(3, max),
                           std::min(max, max-3),
                           std::min(max, max-2),
                           std::min(max, max-1),
                           max};
    for (std::size_t i = 0; i < sizeof(tests)/sizeof(tests[0]); ++i)
    {
        unsigned long long j = tests[i];
        std::bitset<N> v(j);
        assert(j == v.to_ullong());
    }
    { // test values bigger than can fit into the bitset
    const unsigned long long val = 0x55AAAAFFFFAAAA55ULL;
    const bool canFit = N < sizeof(unsigned long long) * CHAR_BIT;
    const unsigned long long mask = canFit ? (1ULL << (canFit ? N : 0)) - 1 : (unsigned long long)(-1); // avoid compiler warnings
    std::bitset<N> v(val);
    assert(v.to_ullong() == (val & mask)); // we shouldn't return bit patterns from outside the limits of the bitset.
    }
}

int main()
{
//     test_to_ullong<0>();
    test_to_ullong<1>();
    test_to_ullong<31>();
    test_to_ullong<32>();
    test_to_ullong<33>();
    test_to_ullong<63>();
    test_to_ullong<64>();
    test_to_ullong<65>();
    test_to_ullong<1000>();
}
