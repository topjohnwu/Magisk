//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>

// logical_not

#include <functional>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

int main()
{
    typedef std::logical_not<int> F;
    const F f = F();
    static_assert((std::is_same<F::argument_type, int>::value), "" );
    static_assert((std::is_same<F::result_type, bool>::value), "" );
    assert(!f(36));
    assert(f(0));
#if TEST_STD_VER > 11
    typedef std::logical_not<> F2;
    const F2 f2 = F2();
    assert(!f2(36));
    assert( f2(0));
    assert(!f2(36L));
    assert( f2(0L));

    constexpr bool foo = std::logical_not<int> () (36);
    static_assert ( !foo, "" );

    constexpr bool bar = std::logical_not<> () (36);
    static_assert ( !bar, "" );
#endif
}
