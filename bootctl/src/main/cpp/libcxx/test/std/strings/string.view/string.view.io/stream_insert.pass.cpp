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
//              const basic_string_view<charT,traits> str);

#include <string_view>
#include <sstream>
#include <cassert>

using std::string_view;
using std::wstring_view;

int main()
{
    {
        std::ostringstream out;
        string_view sv("some text");
        out << sv;
        assert(out.good());
        assert(sv == out.str());
    }
    {
        std::ostringstream out;
        std::string s("some text");
        string_view sv(s);
        out.width(12);
        out << sv;
        assert(out.good());
        assert("   " + s == out.str());
    }
    {
        std::wostringstream out;
        wstring_view sv(L"some text");
        out << sv;
        assert(out.good());
        assert(sv == out.str());
    }
    {
        std::wostringstream out;
        std::wstring s(L"some text");
        wstring_view sv(s);
        out.width(12);
        out << sv;
        assert(out.good());
        assert(L"   " + s == out.str());
    }
}
