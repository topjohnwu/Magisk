//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <list>

// template <class Compare> sort(Compare comp);

#include <list>
#include <functional>
#include <algorithm>  // for is_permutation
#include <cassert>

#include "min_allocator.h"


#ifndef TEST_HAS_NO_EXCEPTIONS
template <typename T>
struct throwingLess {
    throwingLess() : num_(1) {}
    throwingLess(int num) : num_(num) {}

    bool operator() (const T& lhs, const T& rhs) const
    {
    if ( --num_ == 0) throw 1;
    return lhs < rhs;
    }

    mutable int num_;
    };
#endif


int main()
{
    {
    int a1[] = {4, 8, 1, 0, 5, 7, 2, 3, 6, 11, 10, 9};
    int a2[] = {11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
    std::list<int> c1(a1, a1+sizeof(a1)/sizeof(a1[0]));
    c1.sort(std::greater<int>());
    assert(c1 == std::list<int>(a2, a2+sizeof(a2)/sizeof(a2[0])));
    }

//  Test with throwing comparison; make sure that nothing is lost.
//  This is (sort of) LWG #2824
#ifndef TEST_HAS_NO_EXCEPTIONS
    {
    int a1[] = {4, 8, 1, 0, 5, 7, 2, 3, 6, 11, 10, 9};
    const int sz = sizeof(a1)/sizeof(a1[0]);
    for (int i = 0; i < 10; ++i)
        {
        std::list<int> c1(a1, a1 + sz);
        try
            {
            throwingLess<int> comp(i);
            c1.sort(std::cref(comp));
            }
        catch (int) {}
        assert((c1.size() == sz));
        assert((std::is_permutation(c1.begin(), c1.end(), a1)));
        }
    }
#endif

#if TEST_STD_VER >= 11
    {
    int a1[] = {4, 8, 1, 0, 5, 7, 2, 3, 6, 11, 10, 9};
    int a2[] = {11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
    std::list<int, min_allocator<int>> c1(a1, a1+sizeof(a1)/sizeof(a1[0]));
    c1.sort(std::greater<int>());
    assert((c1 == std::list<int, min_allocator<int>>(a2, a2+sizeof(a2)/sizeof(a2[0]))));
    }
#endif
}
