//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>

// greater

#include <functional>
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "pointer_comparison_test_helper.hpp"

int main()
{
    typedef std::greater<int> F;
    const F f = F();
    static_assert((std::is_same<int, F::first_argument_type>::value), "" );
    static_assert((std::is_same<int, F::second_argument_type>::value), "" );
    static_assert((std::is_same<bool, F::result_type>::value), "" );
    assert(!f(36, 36));
    assert(f(36, 6));
    assert(!f(6, 36));
    {
        // test total ordering of int* for greater<int*> and
        // greater<void>.
        do_pointer_comparison_test<int, std::greater>();
    }
#if TEST_STD_VER > 11
    typedef std::greater<> F2;
    const F2 f2 = F2();
    assert(!f2(36, 36));
    assert(f2(36, 6));
    assert(!f2(6, 36));
    assert( f2(36, 6.0));
    assert( f2(36.0, 6));
    assert(!f2(6, 36.0));
    assert(!f2(6.0, 36));

    constexpr bool foo = std::greater<int> () (36, 36);
    static_assert ( !foo, "" );

    constexpr bool bar = std::greater<> () (36.0, 36);
    static_assert ( !bar, "" );
#endif
}
