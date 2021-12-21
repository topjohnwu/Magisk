//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>

// bit_or

#include <functional>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

int main()
{
    typedef std::bit_or<int> F;
    const F f = F();
    static_assert((std::is_same<int, F::first_argument_type>::value), "" );
    static_assert((std::is_same<int, F::second_argument_type>::value), "" );
    static_assert((std::is_same<int, F::result_type>::value), "" );
    assert(f(0xEA95, 0xEA95) == 0xEA95);
    assert(f(0xEA95, 0x58D3) == 0xFAD7);
    assert(f(0x58D3, 0xEA95) == 0xFAD7);
    assert(f(0x58D3, 0) == 0x58D3);
    assert(f(0xFFFF, 0x58D3) == 0xFFFF);
#if TEST_STD_VER > 11
    typedef std::bit_or<> F2;
    const F2 f2 = F2();
    assert(f2(0xEA95, 0xEA95) == 0xEA95);
    assert(f2(0xEA95L, 0xEA95) == 0xEA95);
    assert(f2(0xEA95, 0xEA95L) == 0xEA95);

    assert(f2(0xEA95, 0x58D3) == 0xFAD7);
    assert(f2(0xEA95L, 0x58D3) == 0xFAD7);
    assert(f2(0xEA95, 0x58D3L) == 0xFAD7);

    assert(f2(0x58D3, 0xEA95) == 0xFAD7);
    assert(f2(0x58D3L, 0xEA95) == 0xFAD7);
    assert(f2(0x58D3, 0xEA95L) == 0xFAD7);

    assert(f2(0x58D3, 0) == 0x58D3);
    assert(f2(0x58D3L, 0) == 0x58D3);
    assert(f2(0x58D3, 0L) == 0x58D3);

    assert(f2(0xFFFF, 0x58D3) == 0xFFFF);
    assert(f2(0xFFFFL, 0x58D3) == 0xFFFF);
    assert(f2(0xFFFF, 0x58D3L) == 0xFFFF);

    constexpr int foo = std::bit_or<int> () (0x58D3, 0xEA95);
    static_assert ( foo == 0xFAD7, "" );

    constexpr int bar = std::bit_or<> () (0x58D3L, 0xEA95);
    static_assert ( bar == 0xFAD7, "" );
#endif
}
