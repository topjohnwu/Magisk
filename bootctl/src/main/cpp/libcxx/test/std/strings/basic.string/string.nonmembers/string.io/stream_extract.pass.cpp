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
//   basic_istream<charT,traits>&
//   operator>>(basic_istream<charT,traits>& is,
//              basic_string<charT,traits,Allocator>& str);

#include <string>
#include <sstream>
#include <cassert>

#include "min_allocator.h"

int main()
{
    {
        std::istringstream in("a bc defghij");
        std::string s("initial text");
        in >> s;
        assert(in.good());
        assert(s == "a");
        assert(in.peek() == ' ');
        in >> s;
        assert(in.good());
        assert(s == "bc");
        assert(in.peek() == ' ');
        in.width(3);
        in >> s;
        assert(in.good());
        assert(s == "def");
        assert(in.peek() == 'g');
        in >> s;
        assert(in.eof());
        assert(s == "ghij");
        in >> s;
        assert(in.fail());
    }
    {
        std::wistringstream in(L"a bc defghij");
        std::wstring s(L"initial text");
        in >> s;
        assert(in.good());
        assert(s == L"a");
        assert(in.peek() == L' ');
        in >> s;
        assert(in.good());
        assert(s == L"bc");
        assert(in.peek() == L' ');
        in.width(3);
        in >> s;
        assert(in.good());
        assert(s == L"def");
        assert(in.peek() == L'g');
        in >> s;
        assert(in.eof());
        assert(s == L"ghij");
        in >> s;
        assert(in.fail());
    }
#if TEST_STD_VER >= 11
    {
        typedef std::basic_string<char, std::char_traits<char>, min_allocator<char>> S;
        std::istringstream in("a bc defghij");
        S s("initial text");
        in >> s;
        assert(in.good());
        assert(s == "a");
        assert(in.peek() == ' ');
        in >> s;
        assert(in.good());
        assert(s == "bc");
        assert(in.peek() == ' ');
        in.width(3);
        in >> s;
        assert(in.good());
        assert(s == "def");
        assert(in.peek() == 'g');
        in >> s;
        assert(in.eof());
        assert(s == "ghij");
        in >> s;
        assert(in.fail());
    }
    {
        typedef std::basic_string<wchar_t, std::char_traits<wchar_t>, min_allocator<wchar_t>> S;
        std::wistringstream in(L"a bc defghij");
        S s(L"initial text");
        in >> s;
        assert(in.good());
        assert(s == L"a");
        assert(in.peek() == L' ');
        in >> s;
        assert(in.good());
        assert(s == L"bc");
        assert(in.peek() == L' ');
        in.width(3);
        in >> s;
        assert(in.good());
        assert(s == L"def");
        assert(in.peek() == L'g');
        in >> s;
        assert(in.eof());
        assert(s == L"ghij");
        in >> s;
        assert(in.fail());
    }
#endif
}
