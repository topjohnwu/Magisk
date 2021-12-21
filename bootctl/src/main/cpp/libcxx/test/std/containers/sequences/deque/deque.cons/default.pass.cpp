//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <deque>

// deque()

#include <deque>
#include <cassert>

#include "test_allocator.h"
#include "../../../NotConstructible.h"
#include "min_allocator.h"

template <class T, class Allocator>
void
test()
{
    std::deque<T, Allocator> d;
    assert(d.size() == 0);
#if TEST_STD_VER >= 11
    std::deque<T, Allocator> d1 = {};
    assert(d1.size() == 0);
#endif
}

int main()
{
    test<int, std::allocator<int> >();
    test<NotConstructible, limited_allocator<NotConstructible, 1> >();
#if TEST_STD_VER >= 11
    test<int, min_allocator<int> >();
    test<NotConstructible, min_allocator<NotConstructible> >();
#endif
}
