//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <deque>

// template <class... Args> reference emplace_back(Args&&... args);
// return type is 'reference' in C++17; 'void' before

#include <deque>
#include <cstddef>
#include <cassert>

#include "test_macros.h"
#include "../../../Emplaceable.h"
#include "min_allocator.h"
#include "test_allocator.h"

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
test(C& c1)
{
    typedef typename C::iterator I;
    std::size_t c1_osize = c1.size();
#if TEST_STD_VER > 14
    typedef typename C::reference Ref;
    Ref ref = c1.emplace_back(Emplaceable(1, 2.5));
#else
              c1.emplace_back(Emplaceable(1, 2.5));
#endif
    assert(c1.size() == c1_osize + 1);
    assert(distance(c1.begin(), c1.end())
               == static_cast<std::ptrdiff_t>(c1.size()));
    I i = c1.end();
    assert(*--i == Emplaceable(1, 2.5));
#if TEST_STD_VER > 14
    assert(&(*i) == &ref);
#endif
}

template <class C>
void
testN(int start, int N)
{
    C c1 = make<C>(N, start);
    test(c1);
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
    {
        std::deque<Tag_X, TaggingAllocator<Tag_X>> c;
        c.emplace_back();
        assert(c.size() == 1);
        c.emplace_back(1, 2, 3);
        assert(c.size() == 2);
        c.emplace_front();
        assert(c.size() == 3);
        c.emplace_front(1, 2, 3);
        assert(c.size() == 4);
    }
}
