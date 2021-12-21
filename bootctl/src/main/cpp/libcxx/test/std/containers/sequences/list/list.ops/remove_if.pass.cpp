//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <list>

// template <class Pred> void remove_if(Pred pred);

#include <list>
#include <cassert>
#include <functional>

#include "min_allocator.h"
#include "counting_predicates.hpp"

bool even(int i)
{
    return i % 2 == 0;
}

bool g(int i)
{
    return i < 3;
}

typedef unary_counting_predicate<bool(*)(int), int> Predicate;

int main()
{
    {
    int a1[] = {1, 2, 3, 4};
    int a2[] = {3, 4};
    std::list<int> c(a1, a1+4);
    Predicate cp(g);
    c.remove_if(std::ref(cp));
    assert(c == std::list<int>(a2, a2+2));
    assert(cp.count() == 4);
    }
    {
    int a1[] = {1, 2, 3, 4};
    int a2[] = {1, 3};
    std::list<int> c(a1, a1+4);
    Predicate cp(even);
    c.remove_if(std::ref(cp));
    assert(c == std::list<int>(a2, a2+2));
    assert(cp.count() == 4);
    }
#if TEST_STD_VER >= 11
    {
    int a1[] = {1, 2, 3, 4};
    int a2[] = {3, 4};
    std::list<int, min_allocator<int>> c(a1, a1+4);
    Predicate cp(g);
    c.remove_if(std::ref(cp));
    assert((c == std::list<int, min_allocator<int>>(a2, a2+2)));
    assert(cp.count() == 4);
    }
#endif
}
