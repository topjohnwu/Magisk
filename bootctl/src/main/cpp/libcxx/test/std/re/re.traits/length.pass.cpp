// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <regex>

// template <class charT> struct regex_traits;

// static std::size_t length(const char_type* p);

#include <regex>
#include <cassert>
#include "test_macros.h"

int main()
{
    assert(std::regex_traits<char>::length("") == 0);
    assert(std::regex_traits<char>::length("1") == 1);
    assert(std::regex_traits<char>::length("12") == 2);
    assert(std::regex_traits<char>::length("123") == 3);

    assert(std::regex_traits<wchar_t>::length(L"") == 0);
    assert(std::regex_traits<wchar_t>::length(L"1") == 1);
    assert(std::regex_traits<wchar_t>::length(L"12") == 2);
    assert(std::regex_traits<wchar_t>::length(L"123") == 3);
}
