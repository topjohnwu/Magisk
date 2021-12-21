//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <queue>

// queue(queue&&)
//        noexcept(is_nothrow_move_constructible<container_type>::value);

// This tests a conforming extension


#include <queue>
#include <cassert>

#include "test_macros.h"
#include "MoveOnly.h"

int main()
{
#if defined(_LIBCPP_VERSION)
    {
        typedef std::queue<MoveOnly> C;
        static_assert(std::is_nothrow_move_constructible<C>::value, "");
    }
#endif
}
