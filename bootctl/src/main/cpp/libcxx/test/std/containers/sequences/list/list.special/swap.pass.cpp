//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <list>

// template <class T, class Alloc>
//   void swap(list<T,Alloc>& x, list<T,Alloc>& y);

#include <list>
#include <cassert>
#include "test_allocator.h"
#include "min_allocator.h"

int main()
{
    {
        int a1[] = {1, 3, 7, 9, 10};
        int a2[] = {0, 2, 4, 5, 6, 8, 11};
        std::list<int> c1(a1, a1+sizeof(a1)/sizeof(a1[0]));
        std::list<int> c2(a2, a2+sizeof(a2)/sizeof(a2[0]));
        swap(c1, c2);
        assert(c1 == std::list<int>(a2, a2+sizeof(a2)/sizeof(a2[0])));
        assert(c2 == std::list<int>(a1, a1+sizeof(a1)/sizeof(a1[0])));
    }
    {
        int a1[] = {1, 3, 7, 9, 10};
        int a2[] = {0, 2, 4, 5, 6, 8, 11};
        std::list<int> c1(a1, a1);
        std::list<int> c2(a2, a2+sizeof(a2)/sizeof(a2[0]));
        swap(c1, c2);
        assert(c1 == std::list<int>(a2, a2+sizeof(a2)/sizeof(a2[0])));
        assert(c2.empty());
        assert(distance(c2.begin(), c2.end()) == 0);
    }
    {
        int a1[] = {1, 3, 7, 9, 10};
        int a2[] = {0, 2, 4, 5, 6, 8, 11};
        std::list<int> c1(a1, a1+sizeof(a1)/sizeof(a1[0]));
        std::list<int> c2(a2, a2);
        swap(c1, c2);
        assert(c1.empty());
        assert(distance(c1.begin(), c1.end()) == 0);
        assert(c2 == std::list<int>(a1, a1+sizeof(a1)/sizeof(a1[0])));
    }
    {
        int a1[] = {1, 3, 7, 9, 10};
        int a2[] = {0, 2, 4, 5, 6, 8, 11};
        std::list<int> c1(a1, a1);
        std::list<int> c2(a2, a2);
        swap(c1, c2);
        assert(c1.empty());
        assert(distance(c1.begin(), c1.end()) == 0);
        assert(c2.empty());
        assert(distance(c2.begin(), c2.end()) == 0);
    }
    {
        int a1[] = {1, 3, 7, 9, 10};
        int a2[] = {0, 2, 4, 5, 6, 8, 11};
        typedef test_allocator<int> A;
        std::list<int, A> c1(a1, a1+sizeof(a1)/sizeof(a1[0]), A(1));
        std::list<int, A> c2(a2, a2+sizeof(a2)/sizeof(a2[0]), A(1));
        swap(c1, c2);
        assert((c1 == std::list<int, A>(a2, a2+sizeof(a2)/sizeof(a2[0]))));
        assert(c1.get_allocator() == A(1));
        assert((c2 == std::list<int, A>(a1, a1+sizeof(a1)/sizeof(a1[0]))));
        assert(c2.get_allocator() == A(1));
    }
    {
        int a1[] = {1, 3, 7, 9, 10};
        int a2[] = {0, 2, 4, 5, 6, 8, 11};
        typedef other_allocator<int> A;
        std::list<int, A> c1(a1, a1+sizeof(a1)/sizeof(a1[0]), A(1));
        std::list<int, A> c2(a2, a2+sizeof(a2)/sizeof(a2[0]), A(2));
        swap(c1, c2);
        assert((c1 == std::list<int, A>(a2, a2+sizeof(a2)/sizeof(a2[0]))));
        assert(c1.get_allocator() == A(2));
        assert((c2 == std::list<int, A>(a1, a1+sizeof(a1)/sizeof(a1[0]))));
        assert(c2.get_allocator() == A(1));
    }
#if TEST_STD_VER >= 11
    {
        int a1[] = {1, 3, 7, 9, 10};
        int a2[] = {0, 2, 4, 5, 6, 8, 11};
        std::list<int, min_allocator<int>> c1(a1, a1+sizeof(a1)/sizeof(a1[0]));
        std::list<int, min_allocator<int>> c2(a2, a2+sizeof(a2)/sizeof(a2[0]));
        swap(c1, c2);
        assert((c1 == std::list<int, min_allocator<int>>(a2, a2+sizeof(a2)/sizeof(a2[0]))));
        assert((c2 == std::list<int, min_allocator<int>>(a1, a1+sizeof(a1)/sizeof(a1[0]))));
    }
    {
        int a1[] = {1, 3, 7, 9, 10};
        int a2[] = {0, 2, 4, 5, 6, 8, 11};
        std::list<int, min_allocator<int>> c1(a1, a1);
        std::list<int, min_allocator<int>> c2(a2, a2+sizeof(a2)/sizeof(a2[0]));
        swap(c1, c2);
        assert((c1 == std::list<int, min_allocator<int>>(a2, a2+sizeof(a2)/sizeof(a2[0]))));
        assert(c2.empty());
        assert(distance(c2.begin(), c2.end()) == 0);
    }
    {
        int a1[] = {1, 3, 7, 9, 10};
        int a2[] = {0, 2, 4, 5, 6, 8, 11};
        std::list<int, min_allocator<int>> c1(a1, a1+sizeof(a1)/sizeof(a1[0]));
        std::list<int, min_allocator<int>> c2(a2, a2);
        swap(c1, c2);
        assert(c1.empty());
        assert(distance(c1.begin(), c1.end()) == 0);
        assert((c2 == std::list<int, min_allocator<int>>(a1, a1+sizeof(a1)/sizeof(a1[0]))));
    }
    {
        int a1[] = {1, 3, 7, 9, 10};
        int a2[] = {0, 2, 4, 5, 6, 8, 11};
        std::list<int, min_allocator<int>> c1(a1, a1);
        std::list<int, min_allocator<int>> c2(a2, a2);
        swap(c1, c2);
        assert(c1.empty());
        assert(distance(c1.begin(), c1.end()) == 0);
        assert(c2.empty());
        assert(distance(c2.begin(), c2.end()) == 0);
    }
    {
        int a1[] = {1, 3, 7, 9, 10};
        int a2[] = {0, 2, 4, 5, 6, 8, 11};
        typedef min_allocator<int> A;
        std::list<int, A> c1(a1, a1+sizeof(a1)/sizeof(a1[0]), A());
        std::list<int, A> c2(a2, a2+sizeof(a2)/sizeof(a2[0]), A());
        swap(c1, c2);
        assert((c1 == std::list<int, A>(a2, a2+sizeof(a2)/sizeof(a2[0]))));
        assert(c1.get_allocator() == A());
        assert((c2 == std::list<int, A>(a1, a1+sizeof(a1)/sizeof(a1[0]))));
        assert(c2.get_allocator() == A());
    }
#endif
}
