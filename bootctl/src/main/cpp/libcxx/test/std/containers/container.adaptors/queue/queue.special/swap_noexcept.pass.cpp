//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <queue>

// void swap(queue& c)
//     noexcept(__is_nothrow_swappable<container_type>::value);

// This tests a conforming extension

// UNSUPPORTED: c++98, c++03

#include <queue>
#include <utility>
#include <cassert>

#include "MoveOnly.h"

int main()
{
    {
        typedef std::queue<MoveOnly> C;
        static_assert(noexcept(swap(std::declval<C&>(), std::declval<C&>())), "");
    }
}
