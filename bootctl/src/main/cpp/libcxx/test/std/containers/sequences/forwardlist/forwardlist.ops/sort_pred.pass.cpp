//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <forward_list>

// template <class Compare> void sort(Compare comp);

#include <forward_list>
#include <iterator>
#include <algorithm>
#include <vector>
#include <functional>
#include <random>
#include <cassert>

#include "min_allocator.h"

std::mt19937 randomness;

template <class C>
void test(int N)
{
    typedef typename C::value_type T;
    typedef std::vector<T> V;
    V v;
    for (int i = 0; i < N; ++i)
        v.push_back(i);
    std::shuffle(v.begin(), v.end(), randomness);
    C c(v.begin(), v.end());
    c.sort(std::greater<T>());
    assert(distance(c.begin(), c.end()) == N);
    typename C::const_iterator j = c.begin();
    for (int i = 0; i < N; ++i, ++j)
        assert(*j == N-1-i);
}

int main()
{
    for (int i = 0; i < 40; ++i)
        test<std::forward_list<int> >(i);
#if TEST_STD_VER >= 11
    for (int i = 0; i < 40; ++i)
        test<std::forward_list<int, min_allocator<int>> >(i);
#endif
}
