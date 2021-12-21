//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test bool none() const;

#include <bitset>
#include <type_traits>
#include <cassert>

template <std::size_t N>
void test_none()
{
    std::bitset<N> v;
    v.reset();
    assert(v.none() == true);
    v.set();
    assert(v.none() == (N == 0));
    const bool greater_than_1 = std::integral_constant<bool, (N > 1)>::value; // avoid compiler warnings
    if (greater_than_1)
    {
        v[N/2] = false;
        assert(v.none() == false);
        v.reset();
        v[N/2] = true;
        assert(v.none() == false);
    }
}

int main()
{
    test_none<0>();
    test_none<1>();
    test_none<31>();
    test_none<32>();
    test_none<33>();
    test_none<63>();
    test_none<64>();
    test_none<65>();
    test_none<1000>();
}
