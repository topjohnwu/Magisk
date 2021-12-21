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
//     ostream_joiner(ostream_type& __os, _Delim&& __d);
//     ostream_joiner(ostream_type& __os, const _Delim& __d);

#include <experimental/iterator>
#include <iostream>
#include <string>

#include "test_macros.h"

namespace exp = std::experimental;

int main () {
    const char eight = '8';
    const std::string nine = "9";
    const std::wstring ten = L"10";
    const int eleven = 11;

//  Narrow streams w/rvalues
    { exp::ostream_joiner<char>         oj(std::cout, '8'); }
    { exp::ostream_joiner<std::string>  oj(std::cout, std::string("9")); }
    { exp::ostream_joiner<std::wstring> oj(std::cout, std::wstring(L"10")); }
    { exp::ostream_joiner<int>          oj(std::cout, 11); }

//  Narrow streams w/lvalues
    { exp::ostream_joiner<char>         oj(std::cout, eight); }
    { exp::ostream_joiner<std::string>  oj(std::cout, nine); }
    { exp::ostream_joiner<std::wstring> oj(std::cout, ten); }
    { exp::ostream_joiner<int>          oj(std::cout, eleven); }

//  Wide streams w/rvalues
    { exp::ostream_joiner<char, wchar_t>         oj(std::wcout, '8'); }
    { exp::ostream_joiner<std::string, wchar_t>  oj(std::wcout, std::string("9")); }
    { exp::ostream_joiner<std::wstring, wchar_t> oj(std::wcout, std::wstring(L"10")); }
    { exp::ostream_joiner<int, wchar_t>          oj(std::wcout, 11); }

//  Wide streams w/lvalues
    { exp::ostream_joiner<char, wchar_t>         oj(std::wcout, eight); }
    { exp::ostream_joiner<std::string, wchar_t>  oj(std::wcout, nine); }
    { exp::ostream_joiner<std::wstring, wchar_t> oj(std::wcout, ten); }
    { exp::ostream_joiner<int, wchar_t>          oj(std::wcout, eleven); }

    }
