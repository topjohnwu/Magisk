//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <queue>

// ~queue() // implied noexcept;

// UNSUPPORTED: c++98, c++03

#include <queue>
#include <cassert>

#include "MoveOnly.h"

int main()
{
    {
        typedef std::queue<MoveOnly> C;
        static_assert(std::is_nothrow_destructible<C>::value, "");
    }
}
