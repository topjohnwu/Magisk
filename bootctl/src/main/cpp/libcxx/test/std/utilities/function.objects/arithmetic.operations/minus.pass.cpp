//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>

// minus

#include <functional>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

int main()
{
    typedef std::minus<int> F;
    const F f = F();
    static_assert((std::is_same<int, F::first_argument_type>::value), "" );
    static_assert((std::is_same<int, F::second_argument_type>::value), "" );
    static_assert((std::is_same<int, F::result_type>::value), "" );
    assert(f(3, 2) == 1);
#if TEST_STD_VER > 11
    typedef std::minus<> F2;
    const F2 f2 = F2();
    assert(f2(3,2) == 1);
    assert(f2(3.0, 2) == 1);
    assert(f2(3, 2.5) == 0.5);

    constexpr int foo = std::minus<int> () (3, 2);
    static_assert ( foo == 1, "" );

    constexpr double bar = std::minus<> () (3.0, 2);
    static_assert ( bar == 1.0, "" );
#endif
}
