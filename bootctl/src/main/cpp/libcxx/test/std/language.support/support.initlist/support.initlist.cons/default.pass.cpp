//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// template<class E> class initializer_list;

// initializer_list();

#include <initializer_list>
#include <cassert>

#include "test_macros.h"

struct A {};

int main()
{
    std::initializer_list<A> il;
    assert(il.size() == 0);

#if TEST_STD_VER > 11
    constexpr std::initializer_list<A> il2;
    static_assert(il2.size() == 0, "");
#endif  // TEST_STD_VER > 11
}
