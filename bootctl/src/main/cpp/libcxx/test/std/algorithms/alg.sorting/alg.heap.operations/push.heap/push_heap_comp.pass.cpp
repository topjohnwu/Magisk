//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>

// template<RandomAccessIterator Iter>
//   requires ShuffleIterator<Iter>
//         && LessThanComparable<Iter::value_type>
//   void
//   push_heap(Iter first, Iter last);

#include <algorithm>
#include <functional>
#include <random>
#include <cassert>
#include <memory>

#include "test_macros.h"

struct indirect_less
{
    template <class P>
    bool operator()(const P& x, const P& y)
        {return *x < *y;}
};

std::mt19937 randomness;

void test(int N)
{
    int* ia = new int [N];
    for (int i = 0; i < N; ++i)
        ia[i] = i;
    std::shuffle(ia, ia+N, randomness);
    for (int i = 0; i <= N; ++i)
    {
        std::push_heap(ia, ia+i, std::greater<int>());
        assert(std::is_heap(ia, ia+i, std::greater<int>()));
    }
    delete [] ia;
}

int main()
{
    test(1000);

#if TEST_STD_VER >= 11
    {
    const int N = 1000;
    std::unique_ptr<int>* ia = new std::unique_ptr<int> [N];
    for (int i = 0; i < N; ++i)
        ia[i].reset(new int(i));
    std::shuffle(ia, ia+N, randomness);
    for (int i = 0; i <= N; ++i)
    {
        std::push_heap(ia, ia+i, indirect_less());
        assert(std::is_heap(ia, ia+i, indirect_less()));
    }
    delete [] ia;
    }
#endif
}
