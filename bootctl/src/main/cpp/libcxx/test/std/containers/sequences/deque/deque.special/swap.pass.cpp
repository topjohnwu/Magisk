//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <deque>

// template <class T, class A>
//   void swap(deque<T, A>& x, deque<T, A>& y);

#include <deque>
#include <cassert>
#include "test_allocator.h"
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
    C c(init, 0);
    for (int i = 0; i < init-start; ++i)
        c.pop_back();
    for (int i = 0; i < size; ++i)
        c.push_back(i);
    for (int i = 0; i < start; ++i)
        c.pop_front();
    return c;
}

template <class C>
void testN(int start, int N, int M)
{
    C c1 = make<C>(N, start);
    C c2 = make<C>(M);
    C c1_save = c1;
    C c2_save = c2;
    swap(c1, c2);
    assert(c1 == c2_save);
    assert(c2 == c1_save);
}

int main()
{
    {
        int rng[] = {0, 1, 2, 3, 1023, 1024, 1025, 2047, 2048, 2049};
        const int N = sizeof(rng)/sizeof(rng[0]);
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                for (int k = 0; k < N; ++k)
                    testN<std::deque<int> >(rng[i], rng[j], rng[k]);
    }
    {
        int a1[] = {1, 3, 7, 9, 10};
        int a2[] = {0, 2, 4, 5, 6, 8, 11};
        typedef test_allocator<int> A;
        std::deque<int, A> c1(a1, a1+sizeof(a1)/sizeof(a1[0]), A(1, 1));
        std::deque<int, A> c2(a2, a2+sizeof(a2)/sizeof(a2[0]), A(1, 2));
        swap(c1, c2);
        assert((c1 == std::deque<int, A>(a2, a2+sizeof(a2)/sizeof(a2[0]))));
        assert(c1.get_allocator().get_id() == 1);
        assert((c2 == std::deque<int, A>(a1, a1+sizeof(a1)/sizeof(a1[0]))));
        assert(c2.get_allocator().get_id() == 2);
    }
    {
        int a1[] = {1, 3, 7, 9, 10};
        int a2[] = {0, 2, 4, 5, 6, 8, 11};
        typedef other_allocator<int> A;
        std::deque<int, A> c1(a1, a1+sizeof(a1)/sizeof(a1[0]), A(1));
        std::deque<int, A> c2(a2, a2+sizeof(a2)/sizeof(a2[0]), A(2));
        swap(c1, c2);
        assert((c1 == std::deque<int, A>(a2, a2+sizeof(a2)/sizeof(a2[0]))));
        assert(c1.get_allocator() == A(2));
        assert((c2 == std::deque<int, A>(a1, a1+sizeof(a1)/sizeof(a1[0]))));
        assert(c2.get_allocator() == A(1));
    }
#if TEST_STD_VER >= 11
    {
        int rng[] = {0, 1, 2, 3, 1023, 1024, 1025, 2047, 2048, 2049};
        const int N = sizeof(rng)/sizeof(rng[0]);
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                for (int k = 0; k < N; ++k)
                    testN<std::deque<int, min_allocator<int>> >(rng[i], rng[j], rng[k]);
    }
    {
        int a1[] = {1, 3, 7, 9, 10};
        int a2[] = {0, 2, 4, 5, 6, 8, 11};
        typedef min_allocator<int> A;
        std::deque<int, A> c1(a1, a1+sizeof(a1)/sizeof(a1[0]), A());
        std::deque<int, A> c2(a2, a2+sizeof(a2)/sizeof(a2[0]), A());
        swap(c1, c2);
        assert((c1 == std::deque<int, A>(a2, a2+sizeof(a2)/sizeof(a2[0]))));
        assert(c1.get_allocator() == A());
        assert((c2 == std::deque<int, A>(a1, a1+sizeof(a1)/sizeof(a1[0]))));
        assert(c2.get_allocator() == A());
    }
#endif
}
