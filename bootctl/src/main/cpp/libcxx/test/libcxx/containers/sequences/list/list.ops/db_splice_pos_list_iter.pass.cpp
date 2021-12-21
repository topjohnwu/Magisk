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

// void splice(const_iterator position, list<T,Allocator>& x, iterator i);

#define _LIBCPP_DEBUG 1
#define _LIBCPP_ASSERT(x, m) ((x) ? (void)0 : std::exit(0))

#include <list>
#include <cstdlib>
#include <cassert>

int main()
{
    {
        std::list<int> v1(3);
        std::list<int> v2(3);
        v1.splice(v1.begin(), v2, v1.begin());
        assert(false);
    }
}
