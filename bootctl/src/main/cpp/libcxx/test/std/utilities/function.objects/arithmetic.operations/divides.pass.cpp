//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>

// divides

#include <functional>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

int main()
{
    typedef std::divides<int> F;
    const F f = F();
    static_assert((std::is_same<int, F::first_argument_type>::value), "" );
    static_assert((std::is_same<int, F::second_argument_type>::value), "" );
    static_assert((std::is_same<int, F::result_type>::value), "" );
    assert(f(36, 4) == 9);
#if TEST_STD_VER > 11
    typedef std::divides<> F2;
    const F2 f2 = F2();
    assert(f2(36, 4) == 9);
    assert(f2(36.0, 4) == 9);
    assert(f2(18, 4.0) == 4.5); // exact in binary

    constexpr int foo = std::divides<int> () (3, 2);
    static_assert ( foo == 1, "" );

    constexpr double bar = std::divides<> () (3.0, 2);
    static_assert ( bar == 1.5, "" ); // exact in binary
#endif
}
