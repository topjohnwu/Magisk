//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <stack>

// stack()
//        noexcept(is_nothrow_default_constructible<container_type>::value);

// This tests a conforming extension


#include <stack>
#include <cassert>

#include "test_macros.h"
#include "MoveOnly.h"

int main()
{
#if defined(_LIBCPP_VERSION)
    {
        typedef std::stack<MoveOnly> C;
        static_assert(std::is_nothrow_default_constructible<C>::value, "");
    }
#endif
}
