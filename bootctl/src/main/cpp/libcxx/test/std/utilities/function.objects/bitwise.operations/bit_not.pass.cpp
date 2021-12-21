//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11
// <functional>

// bit_not

#include <functional>
#include <type_traits>
#include <cassert>

int main()
{
    typedef std::bit_not<int> F;
    const F f = F();
    static_assert((std::is_same<F::argument_type, int>::value), "" );
    static_assert((std::is_same<F::result_type, int>::value), "" );
    assert((f(0xEA95) & 0xFFFF ) == 0x156A);
    assert((f(0x58D3) & 0xFFFF ) == 0xA72C);
    assert((f(0)      & 0xFFFF ) == 0xFFFF);
    assert((f(0xFFFF) & 0xFFFF ) == 0);

    typedef std::bit_not<> F2;
    const F2 f2 = F2();
    assert((f2(0xEA95)  & 0xFFFF ) == 0x156A);
    assert((f2(0xEA95L) & 0xFFFF ) == 0x156A);
    assert((f2(0x58D3)  & 0xFFFF ) == 0xA72C);
    assert((f2(0x58D3L) & 0xFFFF ) == 0xA72C);
    assert((f2(0)       & 0xFFFF ) == 0xFFFF);
    assert((f2(0L)      & 0xFFFF ) == 0xFFFF);
    assert((f2(0xFFFF)  & 0xFFFF ) == 0);
    assert((f2(0xFFFFL)  & 0xFFFF ) == 0);

    constexpr int foo = std::bit_not<int> () (0xEA95) & 0xFFFF;
    static_assert ( foo == 0x156A, "" );

    constexpr int bar = std::bit_not<> () (0xEA95) & 0xFFFF;
    static_assert ( bar == 0x156A, "" );
}
