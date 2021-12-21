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

// iterator insert(const_iterator position, value_type&& x);

#define _LIBCPP_DEBUG 1
#define _LIBCPP_ASSERT(x, m) ((x) ? (void)0 : std::exit(0))

#include <list>
#include <cstdlib>
#include <cassert>

int main()
{
    std::list<int> v1(3);
    std::list<int> v2(3);
    v1.insert(v2.begin(), 4);
    assert(false);
}
