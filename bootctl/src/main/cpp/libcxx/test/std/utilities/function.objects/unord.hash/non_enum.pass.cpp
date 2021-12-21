//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11

// <functional>

//  Hashing a struct w/o a defined hash should *not* fail, but it should
// create a type that is not constructible and not callable.
// See also: https://cplusplus.github.io/LWG/lwg-defects.html#2543

#include <functional>
#include <cassert>
#include <type_traits>

#include "test_macros.h"

struct X {};

int main()
{
    using H = std::hash<X>;
    static_assert(!std::is_default_constructible<H>::value, "");
    static_assert(!std::is_copy_constructible<H>::value, "");
    static_assert(!std::is_move_constructible<H>::value, "");
    static_assert(!std::is_copy_assignable<H>::value, "");
    static_assert(!std::is_move_assignable<H>::value, "");
#if TEST_STD_VER > 14
    static_assert(!std::is_invocable<H, X&>::value, "");
    static_assert(!std::is_invocable<H, X const&>::value, "");
#endif
}
