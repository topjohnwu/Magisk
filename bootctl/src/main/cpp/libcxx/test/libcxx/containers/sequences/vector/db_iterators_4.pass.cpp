//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <vector>

// Index iterator out of bounds.

#if _LIBCPP_DEBUG >= 1

#define _LIBCPP_ASSERT(x, m) ((x) ? (void)0 : std::exit(0))

#include <vector>
#include <cassert>
#include <iterator>
#include <exception>
#include <cstdlib>

#include "test_macros.h"
#include "min_allocator.h"

int main()
{
    {
    typedef int T;
    typedef std::vector<T> C;
    C c(1);
    C::iterator i = c.begin();
    assert(i[0] == 0);
    assert(i[1] == 0);
    assert(false);
    }
#if TEST_STD_VER >= 11
    {
    typedef int T;
    typedef std::vector<T, min_allocator<T>> C;
    C c(1);
    C::iterator i = c.begin();
    assert(i[0] == 0);
    assert(i[1] == 0);
    assert(false);
    }
#endif
}

#else

int main()
{
}

#endif
