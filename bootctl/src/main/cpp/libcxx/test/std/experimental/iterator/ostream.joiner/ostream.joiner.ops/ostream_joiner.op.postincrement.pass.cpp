//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11

// <experimental/iterator>
//
// template <class _Delim, class _CharT = char, class _Traits = char_traits<_CharT>>
//   class ostream_joiner;
//
//   ostream_joiner & operator++(int) noexcept
//     returns *this;

#include <experimental/iterator>
#include <iostream>
#include <cassert>

#include "test_macros.h"

namespace exp = std::experimental;

template <class Delim, class CharT, class Traits>
void test ( exp::ostream_joiner<Delim, CharT, Traits> &oj ) {
    static_assert((noexcept(oj++)), "" );
    exp::ostream_joiner<Delim, CharT, Traits> &ret = oj++;
    assert( &ret == &oj );
    }

int main () {

    { exp::ostream_joiner<char>         oj(std::cout, '8');                 test(oj); }
    { exp::ostream_joiner<std::string>  oj(std::cout, std::string("9"));    test(oj); }
    { exp::ostream_joiner<std::wstring> oj(std::cout, std::wstring(L"10")); test(oj); }
    { exp::ostream_joiner<int>          oj(std::cout, 11);                  test(oj); }

    { exp::ostream_joiner<char, wchar_t>         oj(std::wcout, '8');                 test(oj); }
    { exp::ostream_joiner<std::string, wchar_t>  oj(std::wcout, std::string("9"));    test(oj); }
    { exp::ostream_joiner<std::wstring, wchar_t> oj(std::wcout, std::wstring(L"10")); test(oj); }
    { exp::ostream_joiner<int, wchar_t>          oj(std::wcout, 11);                  test(oj); }
    }
