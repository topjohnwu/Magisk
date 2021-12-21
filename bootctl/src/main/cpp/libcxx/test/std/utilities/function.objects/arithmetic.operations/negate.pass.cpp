//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>

// negate

#include <functional>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

int main()
{
    typedef std::negate<int> F;
    const F f = F();
    static_assert((std::is_same<F::argument_type, int>::value), "" );
    static_assert((std::is_same<F::result_type, int>::value), "" );
    assert(f(36) == -36);
#if TEST_STD_VER > 11
    typedef std::negate<> F2;
    const F2 f2 = F2();
    assert(f2(36) == -36);
    assert(f2(36L) == -36);
    assert(f2(36.0) == -36);

    constexpr int foo = std::negate<int> () (3);
    static_assert ( foo == -3, "" );

    constexpr double bar = std::negate<> () (3.0);
    static_assert ( bar == -3.0, "" );
#endif
}
