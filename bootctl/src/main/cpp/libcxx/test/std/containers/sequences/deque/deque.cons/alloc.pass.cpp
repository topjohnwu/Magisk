//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <deque>

// explicit deque(const allocator_type& a);

#include <deque>
#include <cassert>

#include "test_allocator.h"
#include "../../../NotConstructible.h"
#include "min_allocator.h"

template <class T, class Allocator>
void
test(const Allocator& a)
{
    std::deque<T, Allocator> d(a);
    assert(d.size() == 0);
    assert(d.get_allocator() == a);
}

int main()
{
    test<int>(std::allocator<int>());
    test<NotConstructible>(test_allocator<NotConstructible>(3));
#if TEST_STD_VER >= 11
    test<int>(min_allocator<int>());
    test<NotConstructible>(min_allocator<NotConstructible>{});
    test<int>(explicit_allocator<int>());
    test<NotConstructible>(explicit_allocator<NotConstructible>{});
#endif
}
