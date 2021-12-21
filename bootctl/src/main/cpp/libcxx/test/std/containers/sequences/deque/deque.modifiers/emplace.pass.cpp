//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <deque>

// template <class... Args> iterator emplace(const_iterator p, Args&&... args);

// UNSUPPORTED: c++98, c++03

#include <deque>
#include <cassert>
#include <cstddef>

#include "../../../Emplaceable.h"
#include "min_allocator.h"


template <class C>
C
make(int size, int start = 0 )
{
    const int b = 4096 / sizeof(int);
    int init = 0;
    if (start > 0)
    {
        init = (start+1) / b + ((start+1) % b != 0);
        init *= b;
        --init;
    }
    C c(init);
    for (int i = 0; i < init-start; ++i)
        c.pop_back();
    for (int i = 0; i < size; ++i)
        c.push_back(Emplaceable());
    for (int i = 0; i < start; ++i)
        c.pop_front();
    return c;
}

template <class C>
void
test(int P, C& c1)
{
    typedef typename C::const_iterator CI;
    std::size_t c1_osize = c1.size();
    CI i = c1.emplace(c1.begin() + P, Emplaceable(1, 2.5));
    assert(i == c1.begin() + P);
    assert(c1.size() == c1_osize + 1);
    assert(static_cast<std::size_t>(distance(c1.begin(), c1.end())) == c1.size());
    assert(*i == Emplaceable(1, 2.5));
}

template <class C>
void
testN(int start, int N)
{
    for (int i = 0; i <= 3; ++i)
    {
        if (0 <= i && i <= N)
        {
            C c1 = make<C>(N, start);
            test(i, c1);
        }
    }
    for (int i = N/2-1; i <= N/2+1; ++i)
    {
        if (0 <= i && i <= N)
        {
            C c1 = make<C>(N, start);
            test(i, c1);
        }
    }
    for (int i = N - 3; i <= N; ++i)
    {
        if (0 <= i && i <= N)
        {
            C c1 = make<C>(N, start);
            test(i, c1);
        }
    }
}


int main()
{
    {
    int rng[] = {0, 1, 2, 3, 1023, 1024, 1025, 2047, 2048, 2049};
    const int N = sizeof(rng)/sizeof(rng[0]);
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            testN<std::deque<Emplaceable> >(rng[i], rng[j]);
    }
    {
    int rng[] = {0, 1, 2, 3, 1023, 1024, 1025, 2047, 2048, 2049};
    const int N = sizeof(rng)/sizeof(rng[0]);
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            testN<std::deque<Emplaceable, min_allocator<Emplaceable>> >(rng[i], rng[j]);
    }
}
