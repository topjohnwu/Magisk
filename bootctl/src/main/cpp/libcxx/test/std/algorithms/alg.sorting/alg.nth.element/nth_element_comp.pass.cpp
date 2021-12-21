//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>

// template<RandomAccessIterator Iter, StrictWeakOrder<auto, Iter::value_type> Compare>
//   requires ShuffleIterator<Iter>
//         && CopyConstructible<Compare>
//   void
//   nth_element(Iter first, Iter nth, Iter last, Compare comp);

#include <algorithm>
#include <functional>
#include <vector>
#include <random>
#include <cassert>
#include <cstddef>
#include <memory>

#include "test_macros.h"

struct indirect_less
{
    template <class P>
    bool operator()(const P& x, const P& y)
        {return *x < *y;}
};

std::mt19937 randomness;

void
test_one(int N, int M)
{
    assert(N != 0);
    assert(M < N);
    int* array = new int[N];
    for (int i = 0; i < N; ++i)
        array[i] = i;
    std::shuffle(array, array+N, randomness);
    std::nth_element(array, array+M, array+N, std::greater<int>());
    assert(array[M] == N-M-1);
    std::nth_element(array, array+N, array+N, std::greater<int>()); // begin, end, end
    delete [] array;
}

void
test(int N)
{
    test_one(N, 0);
    test_one(N, 1);
    test_one(N, 2);
    test_one(N, 3);
    test_one(N, N/2-1);
    test_one(N, N/2);
    test_one(N, N/2+1);
    test_one(N, N-3);
    test_one(N, N-2);
    test_one(N, N-1);
}

int main()
{
    int d = 0;
    std::nth_element(&d, &d, &d);
    assert(d == 0);
    test(256);
    test(257);
    test(499);
    test(500);
    test(997);
    test(1000);
    test(1009);

#if TEST_STD_VER >= 11
    {
    std::vector<std::unique_ptr<int> > v(1000);
    for (int i = 0; static_cast<std::size_t>(i) < v.size(); ++i)
        v[i].reset(new int(i));
    std::nth_element(v.begin(), v.begin() + v.size()/2, v.end(), indirect_less());
    assert(static_cast<std::size_t>(*v[v.size()/2]) == v.size()/2);
    }
#endif
}
