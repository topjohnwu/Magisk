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

// template <class OutputIter>
//   OutputIter
//   format(OutputIter out, const char_type* fmt_first, const char_type* fmt_last,
//          regex_constants::match_flag_type flags = regex_constants::format_default) const;

#include <regex>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"

int main()
{
    {
        std::match_results<const char*> m;
        const char s[] = "abcdefghijk";
        assert(std::regex_search(s, m, std::regex("cd((e)fg)hi")));

        char out[100] = {0};
        const char fmt[] = "prefix: $`, match: $&, suffix: $', m[1]: $1, m[2]: $2";
        char* r = m.format(output_iterator<char*>(out),
                    fmt, fmt + std::char_traits<char>::length(fmt)).base();
        assert(r == out + 58);
        assert(std::string(out) == "prefix: ab, match: cdefghi, suffix: jk, m[1]: efg, m[2]: e");
    }
    {
        std::match_results<const char*> m;
        const char s[] = "abcdefghijk";
        assert(std::regex_search(s, m, std::regex("cd((e)fg)hi",
                                                  std::regex_constants::nosubs)));

        char out[100] = {0};
        const char fmt[] = "prefix: $`, match: $&, suffix: $', m[1]: $1, m[2]: $2";
        char* r = m.format(output_iterator<char*>(out),
                    fmt, fmt + std::char_traits<char>::length(fmt)).base();
        assert(r == out + 54);
        assert(std::string(out) == "prefix: ab, match: cdefghi, suffix: jk, m[1]: , m[2]: ");
    }
    {
        std::match_results<const char*> m;
        const char s[] = "abcdefghijk";
        assert(std::regex_search(s, m, std::regex("cdefghi")));

        char out[100] = {0};
        const char fmt[] = "prefix: $`, match: $&, suffix: $', m[1]: $1, m[2]: $2";
        char* r = m.format(output_iterator<char*>(out),
                    fmt, fmt + std::char_traits<char>::length(fmt)).base();
        assert(r == out + 54);
        assert(std::string(out) == "prefix: ab, match: cdefghi, suffix: jk, m[1]: , m[2]: ");
    }
    {
        std::match_results<const char*> m;
        const char s[] = "abcdefghijk";
        assert(std::regex_search(s, m, std::regex("cd((e)fg)hi")));

        char out[100] = {0};
        const char fmt[] = "prefix: $`, match: $&, suffix: $', m[1]: $1, m[2]: $2";
        char* r = m.format(output_iterator<char*>(out),
                    fmt, fmt + std::char_traits<char>::length(fmt),
                    std::regex_constants::format_sed).base();
        assert(r == out + 59);
        assert(std::string(out) == "prefix: $`, match: $cdefghi, suffix: $', m[1]: $1, m[2]: $2");
    }
    {
        std::match_results<const char*> m;
        const char s[] = "abcdefghijk";
        assert(std::regex_search(s, m, std::regex("cd((e)fg)hi")));

        char out[100] = {0};
        const char fmt[] = "match: &, m[1]: \\1, m[2]: \\2";
        char* r = m.format(output_iterator<char*>(out),
                    fmt, fmt + std::char_traits<char>::length(fmt),
                    std::regex_constants::format_sed).base();
        assert(r == out + 34);
        assert(std::string(out) == "match: cdefghi, m[1]: efg, m[2]: e");
    }
    {
        std::match_results<const char*> m;
        const char s[] = "abcdefghijk";
        assert(std::regex_search(s, m, std::regex("cd((e)fg)hi",
                                                  std::regex_constants::nosubs)));

        char out[100] = {0};
        const char fmt[] = "match: &, m[1]: \\1, m[2]: \\2";
        char* r = m.format(output_iterator<char*>(out),
                    fmt, fmt + std::char_traits<char>::length(fmt),
                    std::regex_constants::format_sed).base();
        assert(r == out + 30);
        assert(std::string(out) == "match: cdefghi, m[1]: , m[2]: ");
    }
    {
        std::match_results<const char*> m;
        const char s[] = "abcdefghijk";
        assert(std::regex_search(s, m, std::regex("cdefghi")));

        char out[100] = {0};
        const char fmt[] = "match: &, m[1]: \\1, m[2]: \\2";
        char* r = m.format(output_iterator<char*>(out),
                    fmt, fmt + std::char_traits<char>::length(fmt),
                    std::regex_constants::format_sed).base();
        assert(r == out + 30);
        assert(std::string(out) == "match: cdefghi, m[1]: , m[2]: ");
    }

    {
        std::match_results<const wchar_t*> m;
        const wchar_t s[] = L"abcdefghijk";
        assert(std::regex_search(s, m, std::wregex(L"cd((e)fg)hi")));

        wchar_t out[100] = {0};
        const wchar_t fmt[] = L"prefix: $`, match: $&, suffix: $', m[1]: $1, m[2]: $2";
        wchar_t* r = m.format(output_iterator<wchar_t*>(out),
                    fmt, fmt + std::char_traits<wchar_t>::length(fmt)).base();
        assert(r == out + 58);
        assert(std::wstring(out) == L"prefix: ab, match: cdefghi, suffix: jk, m[1]: efg, m[2]: e");
    }
    {
        std::match_results<const wchar_t*> m;
        const wchar_t s[] = L"abcdefghijk";
        assert(std::regex_search(s, m, std::wregex(L"cd((e)fg)hi")));

        wchar_t out[100] = {0};
        const wchar_t fmt[] = L"prefix: $`, match: $&, suffix: $', m[1]: $1, m[2]: $2";
        wchar_t* r = m.format(output_iterator<wchar_t*>(out),
                    fmt, fmt + std::char_traits<wchar_t>::length(fmt),
                    std::regex_constants::format_sed).base();
        assert(r == out + 59);
        assert(std::wstring(out) == L"prefix: $`, match: $cdefghi, suffix: $', m[1]: $1, m[2]: $2");
    }
    {
        std::match_results<const wchar_t*> m;
        const wchar_t s[] = L"abcdefghijk";
        assert(std::regex_search(s, m, std::wregex(L"cd((e)fg)hi")));

        wchar_t out[100] = {0};
        const wchar_t fmt[] = L"match: &, m[1]: \\1, m[2]: \\2";
        wchar_t* r = m.format(output_iterator<wchar_t*>(out),
                    fmt, fmt + std::char_traits<wchar_t>::length(fmt),
                    std::regex_constants::format_sed).base();
        assert(r == out + 34);
        assert(std::wstring(out) == L"match: cdefghi, m[1]: efg, m[2]: e");
    }
}
