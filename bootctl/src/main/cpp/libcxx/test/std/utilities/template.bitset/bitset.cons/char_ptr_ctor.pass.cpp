//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// template <class charT>
//     explicit bitset(const charT* str,
//                     typename basic_string<charT>::size_type n = basic_string<charT>::npos,
//                     charT zero = charT('0'), charT one = charT('1'));

#include <bitset>
#include <cassert>
#include <algorithm> // for 'min' and 'max'
#include <stdexcept> // for 'invalid_argument'

#include "test_macros.h"

#if defined(TEST_COMPILER_C1XX)
#pragma warning(disable: 6294) // Ill-defined for-loop:  initial condition does not satisfy test.  Loop body not executed.
#endif

template <std::size_t N>
void test_char_pointer_ctor()
{
    {
#ifndef TEST_HAS_NO_EXCEPTIONS
        try {
            std::bitset<N> v("xxx1010101010xxxx");
            assert(false);
        }
        catch (std::invalid_argument&) {}
#endif
    }

    {
    const char str[] ="1010101010";
    std::bitset<N> v(str);
    std::size_t M = std::min<std::size_t>(N, 10);
    for (std::size_t i = 0; i < M; ++i)
        assert(v[i] == (str[M - 1 - i] == '1'));
    for (std::size_t i = 10; i < N; ++i)
        assert(v[i] == false);
    }
}

int main()
{
    test_char_pointer_ctor<0>();
    test_char_pointer_ctor<1>();
    test_char_pointer_ctor<31>();
    test_char_pointer_ctor<32>();
    test_char_pointer_ctor<33>();
    test_char_pointer_ctor<63>();
    test_char_pointer_ctor<64>();
    test_char_pointer_ctor<65>();
    test_char_pointer_ctor<1000>();
}
