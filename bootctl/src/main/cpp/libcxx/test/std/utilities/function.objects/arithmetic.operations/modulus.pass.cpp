//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>

// modulus

#include <functional>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

int main()
{
    typedef std::modulus<int> F;
    const F f = F();
    static_assert((std::is_same<int, F::first_argument_type>::value), "" );
    static_assert((std::is_same<int, F::second_argument_type>::value), "" );
    static_assert((std::is_same<int, F::result_type>::value), "" );
    assert(f(36, 8) == 4);
#if TEST_STD_VER > 11
    typedef std::modulus<> F2;
    const F2 f2 = F2();
    assert(f2(36, 8) == 4);
    assert(f2(36L, 8) == 4);
    assert(f2(36, 8L) == 4);

    constexpr int foo = std::modulus<int> () (3, 2);
    static_assert ( foo == 1, "" );

    constexpr int bar = std::modulus<> () (3L, 2);
    static_assert ( bar == 1, "" );
#endif
}
