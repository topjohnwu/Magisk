//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <queue>

// priority_queue& operator=(priority_queue&& c)
//     noexcept(is_nothrow_move_assignable<container_type>::value &&
//              is_nothrow_move_assignable<Compare>::value);

// This tests a conforming extension

// UNSUPPORTED: c++98, c++03

#include <queue>
#include <cassert>

#include "MoveOnly.h"

int main()
{
    {
        typedef std::priority_queue<MoveOnly> C;
        static_assert(std::is_nothrow_move_assignable<C>::value, "");
    }
}
