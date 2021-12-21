//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test size_t count() const;

#include <bitset>
#include <cassert>

template <std::size_t N>
void test_size()
{
    const std::bitset<N> v;
    assert(v.size() == N);
}

int main()
{
    test_size<0>();
    test_size<1>();
    test_size<31>();
    test_size<32>();
    test_size<33>();
    test_size<63>();
    test_size<64>();
    test_size<65>();
    test_size<1000>();
}
