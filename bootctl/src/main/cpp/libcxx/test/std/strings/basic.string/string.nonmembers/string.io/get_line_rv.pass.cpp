//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <string>

// template<class charT, class traits, class Allocator>
//   basic_istream<charT,traits>&
//   getline(basic_istream<charT,traits>&& is,
//           basic_string<charT,traits,Allocator>& str);

#include <string>
#include <sstream>
#include <cassert>

#include "min_allocator.h"

int main()
{
    {
        std::string s("initial text");
        getline(std::istringstream(" abc\n  def\n   ghij"), s);
        assert(s == " abc");
    }
    {
        std::wstring s(L"initial text");
        getline(std::wistringstream(L" abc\n  def\n   ghij"), s);
        assert(s == L" abc");
    }
    {
        typedef std::basic_string<char, std::char_traits<char>, min_allocator<char>> S;
        S s("initial text");
        getline(std::istringstream(" abc\n  def\n   ghij"), s);
        assert(s == " abc");
    }
    {
        typedef std::basic_string<wchar_t, std::char_traits<wchar_t>, min_allocator<wchar_t>> S;
        S s(L"initial text");
        getline(std::wistringstream(L" abc\n  def\n   ghij"), s);
        assert(s == L" abc");
    }
}
