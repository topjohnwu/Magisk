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
//   stable_sort(Iter first, Iter last, Compare comp);

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

struct first_only
{
    bool operator()(const std::pair<int, int>& x, const std::pair<int, int>& y)
    {
        return x.first < y.first;
    }
};

void test()
{
    typedef std::pair<int, int> P;
    const int N = 1000;
    const int M = 10;
    std::vector<P> v(N);
    int x = 0;
    int ver = 0;
    for (int i = 0; i < N; ++i)
    {
        v[i] = P(x, ver);
        if (++x == M)
        {
            x = 0;
            ++ver;
        }
    }
    for (int i = 0; i < N - M; i += M)
    {
        std::shuffle(v.begin() + i, v.begin() + i + M, randomness);
    }
    std::stable_sort(v.begin(), v.end(), first_only());
    assert(std::is_sorted(v.begin(), v.end()));
}

int main()
{
    test();

#if TEST_STD_VER >= 11
    {
    std::vector<std::unique_ptr<int> > v(1000);
    for (int i = 0; static_cast<std::size_t>(i) < v.size(); ++i)
        v[i].reset(new int(i));
    std::stable_sort(v.begin(), v.end(), indirect_less());
    assert(std::is_sorted(v.begin(), v.end(), indirect_less()));
    assert(*v[0] == 0);
    assert(*v[1] == 1);
    assert(*v[2] == 2);
    }
#endif
}
