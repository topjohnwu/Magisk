//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <regex>

// class match_results<BidirectionalIterator, Allocator>

// template <class ST, class SA>
//   basic_string<char_type, ST, SA>
//   format(const basic_string<char_type, ST, SA>& fmt,
//          regex_constants::match_flag_type flags = regex_constants::format_default) const;

#include <iostream>

#include <regex>
#include <cassert>

#include "test_macros.h"
#include "test_allocator.h"

int main()
{
    typedef std::basic_string<char, std::char_traits<char>, test_allocator<char> > nstr;
    typedef std::basic_string<wchar_t, std::char_traits<wchar_t>, test_allocator<wchar_t> > wstr;
    {
        std::match_results<const char*> m;
        const char s[] = "abcdefghijk";
        assert(std::regex_search(s, m, std::regex("cd((e)fg)hi")));

        nstr fmt("prefix: $`, match: $&, suffix: $', m[1]: $1, m[2]: $2");
        nstr out = m.format(fmt);
        assert(out == "prefix: ab, match: cdefghi, suffix: jk, m[1]: efg, m[2]: e");
    }
    {
        std::match_results<const char*> m;
        const char s[] = "abcdefghijk";
        assert(std::regex_search(s, m, std::regex("cd((e)fg)hi")));

        nstr fmt("prefix: $`, match: $&, suffix: $', m[1]: $1, m[2]: $2");
        nstr out = m.format(fmt, std::regex_constants::format_sed);
        assert(out == "prefix: $`, match: $cdefghi, suffix: $', m[1]: $1, m[2]: $2");
    }
    {
        std::match_results<const char*> m;
        const char s[] = "abcdefghijk";
        assert(std::regex_search(s, m, std::regex("cd((e)fg)hi")));

        nstr fmt("match: &, m[1]: \\1, m[2]: \\2");
        nstr out = m.format(fmt, std::regex_constants::format_sed);
        assert(out == "match: cdefghi, m[1]: efg, m[2]: e");
    }

    {
        std::match_results<const wchar_t*> m;
        const wchar_t s[] = L"abcdefghijk";
        assert(std::regex_search(s, m, std::wregex(L"cd((e)fg)hi")));

        wstr fmt(L"prefix: $`, match: $&, suffix: $', m[1]: $1, m[2]: $2");
        wstr out = m.format(fmt);
        assert(out == L"prefix: ab, match: cdefghi, suffix: jk, m[1]: efg, m[2]: e");
    }
    {
        std::match_results<const wchar_t*> m;
        const wchar_t s[] = L"abcdefghijk";
        assert(std::regex_search(s, m, std::wregex(L"cd((e)fg)hi")));

        wstr fmt(L"prefix: $`, match: $&, suffix: $', m[1]: $1, m[2]: $2");
        wstr out = m.format(fmt, std::regex_constants::format_sed);
        assert(out == L"prefix: $`, match: $cdefghi, suffix: $', m[1]: $1, m[2]: $2");
    }
    {
        std::match_results<const wchar_t*> m;
        const wchar_t s[] = L"abcdefghijk";
        assert(std::regex_search(s, m, std::wregex(L"cd((e)fg)hi")));

        wstr fmt(L"match: &, m[1]: \\1, m[2]: \\2");
        wstr out = m.format(fmt, std::regex_constants::format_sed);
        assert(out == L"match: cdefghi, m[1]: efg, m[2]: e");
    }
}
