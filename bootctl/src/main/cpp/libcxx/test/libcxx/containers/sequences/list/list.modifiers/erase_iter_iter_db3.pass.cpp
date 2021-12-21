//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Can't test the system lib because this test enables debug mode
// UNSUPPORTED: with_system_cxx_lib

// <list>

// Call erase(const_iterator first, const_iterator last); with both iterators from another container

#define _LIBCPP_DEBUG 1
#define _LIBCPP_ASSERT(x, m) ((x) ? (void)0 : std::exit(0))

#include <list>
#include <cassert>
#include <cstdlib>

int main()
{
    int a1[] = {1, 2, 3};
    std::list<int> l1(a1, a1+3);
    std::list<int> l2(a1, a1+3);
    std::list<int>::iterator i = l1.erase(l2.cbegin(), next(l2.cbegin()));
    assert(false);
}
