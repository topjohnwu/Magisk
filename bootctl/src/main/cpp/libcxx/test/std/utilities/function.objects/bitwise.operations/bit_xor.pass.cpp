//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>

// bit_xor

#include <functional>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

int main()
{
    {
    typedef std::bit_xor<int> F;
    const F f = F();
    static_assert((std::is_same<int, F::first_argument_type>::value), "" );
    static_assert((std::is_same<int, F::second_argument_type>::value), "" );
    static_assert((std::is_same<int, F::result_type>::value), "" );
    assert(f(0xEA95, 0xEA95) == 0);
    assert(f(0xEA95, 0x58D3) == 0xB246);
    assert(f(0x58D3, 0xEA95) == 0xB246);
    assert(f(0x58D3, 0) == 0x58D3);
    assert(f(0xFFFF, 0x58D3) == 0xA72C);
    }
#if TEST_STD_VER > 11
    {
    typedef std::bit_xor<> F2;
    const F2 f = F2();
    assert(f(0xEA95, 0xEA95) == 0);
    assert(f(0xEA95L, 0xEA95) == 0);
    assert(f(0xEA95, 0xEA95L) == 0);

    assert(f(0xEA95, 0x58D3) == 0xB246);
    assert(f(0xEA95L, 0x58D3) == 0xB246);
    assert(f(0xEA95, 0x58D3L) == 0xB246);

    assert(f(0x58D3, 0xEA95) == 0xB246);
    assert(f(0x58D3L, 0xEA95) == 0xB246);
    assert(f(0x58D3, 0xEA95L) == 0xB246);

    assert(f(0x58D3, 0) == 0x58D3);
    assert(f(0x58D3L, 0) == 0x58D3);
    assert(f(0x58D3, 0L) == 0x58D3);

    assert(f(0xFFFF, 0x58D3) == 0xA72C);
    assert(f(0xFFFFL, 0x58D3) == 0xA72C);
    assert(f(0xFFFF, 0x58D3L) == 0xA72C);

    constexpr int foo = std::bit_xor<int> () (0x58D3, 0xEA95);
    static_assert ( foo == 0xB246, "" );

    constexpr int bar = std::bit_xor<> () (0x58D3L, 0xEA95);
    static_assert ( bar == 0xB246, "" );
    }
#endif
}
