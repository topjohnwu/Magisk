//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// template<class charT, class traits, class Allocator>
//   basic_ostream<charT, traits>&
//   operator<<(basic_ostream<charT, traits>& os,
//              const basic_string<charT,traits,Allocator>& str);

#include <string>
#include <sstream>
#include <cassert>

#include "min_allocator.h"

int main()
{
    {
        std::ostringstream out;
        std::string s("some text");
        out << s;
        assert(out.good());
        assert(s == out.str());
    }
    {
        std::ostringstream out;
        std::string s("some text");
        out.width(12);
        out << s;
        assert(out.good());
        assert("   " + s == out.str());
    }
    {
        std::wostringstream out;
        std::wstring s(L"some text");
        out << s;
        assert(out.good());
        assert(s == out.str());
    }
    {
        std::wostringstream out;
        std::wstring s(L"some text");
        out.width(12);
        out << s;
        assert(out.good());
        assert(L"   " + s == out.str());
    }
#if TEST_STD_VER >= 11
    {
        typedef std::basic_string<char, std::char_traits<char>, min_allocator<char>> S;
        std::basic_ostringstream<S::value_type, S::traits_type, S::allocator_type> out;
        S s("some text");
        out << s;
        assert(out.good());
        assert(s == out.str());
    }
    {
        typedef std::basic_string<char, std::char_traits<char>, min_allocator<char>> S;
        std::basic_ostringstream<S::value_type, S::traits_type, S::allocator_type> out;
        S s("some text");
        out.width(12);
        out << s;
        assert(out.good());
        assert("   " + s == out.str());
    }
    {
        typedef std::basic_string<wchar_t, std::char_traits<wchar_t>, min_allocator<wchar_t>> S;
        std::basic_ostringstream<S::value_type, S::traits_type, S::allocator_type> out;
        S s(L"some text");
        out << s;
        assert(out.good());
        assert(s == out.str());
    }
    {
        typedef std::basic_string<wchar_t, std::char_traits<wchar_t>, min_allocator<wchar_t>> S;
        std::basic_ostringstream<S::value_type, S::traits_type, S::allocator_type> out;
        S s(L"some text");
        out.width(12);
        out << s;
        assert(out.good());
        assert(L"   " + s == out.str());
    }
#endif
}
