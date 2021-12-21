//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <list>

// iterator insert(const_iterator position, const value_type& x);

#include <list>
#include <cstdlib>
#include <cassert>

#include "test_macros.h"
#include "min_allocator.h"
#include "count_new.hpp"

template <class List>
void test()
{
    int a1[] = {1, 2, 3};
    int a2[] = {1, 4, 2, 3};
    List l1(a1, a1+3);
    typename List::iterator i = l1.insert(next(l1.cbegin()), 4);
    assert(i == next(l1.begin()));
    assert(l1.size() == 4);
    assert(distance(l1.begin(), l1.end()) == 4);
    assert(l1 == List(a2, a2+4));

#if !defined(TEST_HAS_NO_EXCEPTIONS) && !defined(DISABLE_NEW_COUNT)
    globalMemCounter.throw_after = 0;
    int save_count = globalMemCounter.outstanding_new;
    try
    {
        i = l1.insert(i, 5);
        assert(false);
    }
    catch (...)
    {
    }
    assert(globalMemCounter.checkOutstandingNewEq(save_count));
    assert(l1 == List(a2, a2+4));
#endif
}

int main()
{
    test<std::list<int> >();
#if TEST_STD_VER >= 11
    test<std::list<int, min_allocator<int>>>();
#endif
}
