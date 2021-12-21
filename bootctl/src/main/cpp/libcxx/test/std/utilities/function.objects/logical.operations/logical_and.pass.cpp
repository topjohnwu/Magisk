//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>

// logical_and

#include <functional>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

int main()
{
    typedef std::logical_and<int> F;
    const F f = F();
    static_assert((std::is_same<int, F::first_argument_type>::value), "" );
    static_assert((std::is_same<int, F::second_argument_type>::value), "" );
    static_assert((std::is_same<bool, F::result_type>::value), "" );
    assert(f(36, 36));
    assert(!f(36, 0));
    assert(!f(0, 36));
    assert(!f(0, 0));
#if TEST_STD_VER > 11
    typedef std::logical_and<> F2;
    const F2 f2 = F2();
    assert( f2(36, 36));
    assert( f2(36, 36L));
    assert( f2(36L, 36));
    assert(!f2(36, 0));
    assert(!f2(0, 36));
    assert( f2(36, 36L));
    assert(!f2(36, 0L));
    assert(!f2(0, 36L));
    assert( f2(36L, 36));
    assert(!f2(36L, 0));
    assert(!f2(0L, 36));

    constexpr bool foo = std::logical_and<int> () (36, 36);
    static_assert ( foo, "" );

    constexpr bool bar = std::logical_and<> () (36.0, 36);
    static_assert ( bar, "" );
#endif
}
