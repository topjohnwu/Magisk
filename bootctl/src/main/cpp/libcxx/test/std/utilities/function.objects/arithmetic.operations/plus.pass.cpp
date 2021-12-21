//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>

// plus

#include <functional>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

int main()
{
    typedef std::plus<int> F;
    const F f = F();
    static_assert((std::is_same<int, F::first_argument_type>::value), "" );
    static_assert((std::is_same<int, F::second_argument_type>::value), "" );
    static_assert((std::is_same<int, F::result_type>::value), "" );
    assert(f(3, 2) == 5);
#if TEST_STD_VER > 11
    typedef std::plus<> F2;
    const F2 f2 = F2();
    assert(f2(3,2) == 5);
    assert(f2(3.0, 2) == 5);
    assert(f2(3, 2.5) == 5.5);

    constexpr int foo = std::plus<int> () (3, 2);
    static_assert ( foo == 5, "" );

    constexpr double bar = std::plus<> () (3.0, 2);
    static_assert ( bar == 5.0, "" );
#endif
}
