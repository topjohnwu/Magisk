//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// NetBSD does not support LC_COLLATE at the moment
// XFAIL: netbsd

// REQUIRES: locale.cs_CZ.ISO8859-2

// <regex>

// template <class BidirectionalIterator, class Allocator, class charT, class traits>
//     bool
//     regex_match(BidirectionalIterator first, BidirectionalIterator last,
//                  match_results<BidirectionalIterator, Allocator>& m,
//                  const basic_regex<charT, traits>& e,
//                  regex_constants::match_flag_type flags = regex_constants::match_default);

// TODO: investigation needed
// XFAIL: linux-gnu

#include <regex>
#include <cassert>
#include "test_macros.h"
#include "test_iterators.h"

#include "platform_support.h" // locale name macros

int main()
{
    {
        std::cmatch m;
        const char s[] = "a";
        assert(std::regex_match(s, m, std::regex("a")));
        assert(m.size() == 1);
        assert(!m.empty());
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s+1);
        assert(m.length(0) == 1);
        assert(m.position(0) == 0);
        assert(m.str(0) == "a");
    }
    {
        std::cmatch m;
        const char s[] = "ab";
        assert(std::regex_match(s, m, std::regex("ab")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s+2);
        assert(m.length(0) == 2);
        assert(m.position(0) == 0);
        assert(m.str(0) == "ab");
    }
    {
        std::cmatch m;
        const char s[] = "ab";
        assert(!std::regex_match(s, m, std::regex("ba")));
        assert(m.size() == 0);
        assert(m.empty());
    }
    {
        std::cmatch m;
        const char s[] = "aab";
        assert(!std::regex_match(s, m, std::regex("ab")));
        assert(m.size() == 0);
    }
    {
        std::cmatch m;
        const char s[] = "aab";
        assert(!std::regex_match(s, m, std::regex("ab"),
                                            std::regex_constants::match_continuous));
        assert(m.size() == 0);
    }
    {
        std::cmatch m;
        const char s[] = "abcd";
        assert(!std::regex_match(s, m, std::regex("bc")));
        assert(m.size() == 0);
    }
    {
        std::cmatch m;
        const char s[] = "abbc";
        assert(std::regex_match(s, m, std::regex("ab*c")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s+4);
        assert(m.length(0) == 4);
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::cmatch m;
        const char s[] = "ababc";
        assert(std::regex_match(s, m, std::regex("(ab)*c")));
        assert(m.size() == 2);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s+5);
        assert(m.length(0) == 5);
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
        assert(m.length(1) == 2);
        assert(m.position(1) == 2);
        assert(m.str(1) == "ab");
    }
    {
        std::cmatch m;
        const char s[] = "abcdefghijk";
        assert(!std::regex_match(s, m, std::regex("cd((e)fg)hi")));
        assert(m.size() == 0);
    }
    {
        std::cmatch m;
        const char s[] = "abc";
        assert(std::regex_match(s, m, std::regex("^abc")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s+3);
        assert(m.length(0) == 3);
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::cmatch m;
        const char s[] = "abcd";
        assert(!std::regex_match(s, m, std::regex("^abc")));
        assert(m.size() == 0);
    }
    {
        std::cmatch m;
        const char s[] = "aabc";
        assert(!std::regex_match(s, m, std::regex("^abc")));
        assert(m.size() == 0);
    }
    {
        std::cmatch m;
        const char s[] = "abc";
        assert(std::regex_match(s, m, std::regex("abc$")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s+3);
        assert(m.length(0) == 3);
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::cmatch m;
        const char s[] = "efabc";
        assert(!std::regex_match(s, m, std::regex("abc$")));
        assert(m.size() == 0);
    }
    {
        std::cmatch m;
        const char s[] = "efabcg";
        assert(!std::regex_match(s, m, std::regex("abc$")));
        assert(m.size() == 0);
    }
    {
        std::cmatch m;
        const char s[] = "abc";
        assert(std::regex_match(s, m, std::regex("a.c")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s+3);
        assert(m.length(0) == 3);
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::cmatch m;
        const char s[] = "acc";
        assert(std::regex_match(s, m, std::regex("a.c")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s+3);
        assert(m.length(0) == 3);
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::cmatch m;
        const char s[] = "acc";
        assert(std::regex_match(s, m, std::regex("a.c")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s+3);
        assert(m.length(0) == 3);
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::cmatch m;
        const char s[] = "abcdef";
        assert(std::regex_match(s, m, std::regex("(.*).*")));
        assert(m.size() == 2);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s+6);
        assert(m.length(0) == 6);
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
        assert(m.length(1) == 6);
        assert(m.position(1) == 0);
        assert(m.str(1) == s);
    }
    {
        std::cmatch m;
        const char s[] = "bc";
        assert(!std::regex_match(s, m, std::regex("(a*)*")));
        assert(m.size() == 0);
    }
    {
        std::cmatch m;
        const char s[] = "abbc";
        assert(!std::regex_match(s, m, std::regex("ab{3,5}c")));
        assert(m.size() == 0);
    }
    {
        std::cmatch m;
        const char s[] = "abbbc";
        assert(std::regex_match(s, m, std::regex("ab{3,5}c")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) >= 0 && static_cast<size_t>(m.length(0)) == std::char_traits<char>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::cmatch m;
        const char s[] = "abbbbc";
        assert(std::regex_match(s, m, std::regex("ab{3,5}c")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) >= 0 && static_cast<size_t>(m.length(0)) == std::char_traits<char>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::cmatch m;
        const char s[] = "abbbbbc";
        assert(std::regex_match(s, m, std::regex("ab{3,5}c")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) >= 0 && static_cast<size_t>(m.length(0)) == std::char_traits<char>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::cmatch m;
        const char s[] = "adefc";
        assert(!std::regex_match(s, m, std::regex("ab{3,5}c")));
        assert(m.size() == 0);
    }
    {
        std::cmatch m;
        const char s[] = "abbbbbbc";
        assert(!std::regex_match(s, m, std::regex("ab{3,5}c")));
        assert(m.size() == 0);
    }
    {
        std::cmatch m;
        const char s[] = "adec";
        assert(!std::regex_match(s, m, std::regex("a.{3,5}c")));
        assert(m.size() == 0);
    }
    {
        std::cmatch m;
        const char s[] = "adefc";
        assert(std::regex_match(s, m, std::regex("a.{3,5}c")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) >= 0 && static_cast<size_t>(m.length(0)) == std::char_traits<char>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::cmatch m;
        const char s[] = "adefgc";
        assert(std::regex_match(s, m, std::regex("a.{3,5}c")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) >= 0 && static_cast<size_t>(m.length(0)) == std::char_traits<char>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::cmatch m;
        const char s[] = "adefghc";
        assert(std::regex_match(s, m, std::regex("a.{3,5}c")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) >= 0 && static_cast<size_t>(m.length(0)) == std::char_traits<char>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::cmatch m;
        const char s[] = "adefghic";
        assert(!std::regex_match(s, m, std::regex("a.{3,5}c")));
        assert(m.size() == 0);
    }
    {
        std::cmatch m;
        // http://www.open-std.org/jtc1/sc22/wg21/docs/lwg-defects.html#2273
        const char s[] = "tournament";
        assert(std::regex_match(s, m, std::regex("tour|to|tournament")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) >= 0 && static_cast<size_t>(m.length(0)) == std::char_traits<char>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::cmatch m;
        // http://www.open-std.org/jtc1/sc22/wg21/docs/lwg-defects.html#2273
        const char s[] = "tournamenttotour";
        assert(
            std::regex_match(s, m, std::regex("(tour|to|tournament)+",
                                              std::regex_constants::nosubs)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) >= 0 && static_cast<size_t>(m.length(0)) == std::char_traits<char>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::cmatch m;
        const char s[] = "ttotour";
        assert(std::regex_match(s, m, std::regex("(tour|to|t)+")));
        assert(m.size() == 2);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) >= 0 && static_cast<size_t>(m.length(0)) == std::char_traits<char>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
        assert(m.length(1) == 4);
        assert(m.position(1) == 3);
        assert(m.str(1) == "tour");
    }
    {
        std::cmatch m;
        const char s[] = "-ab,ab-";
        assert(!std::regex_match(s, m, std::regex("-(.*),\1-")));
        assert(m.size() == 0);
    }
    {
        std::cmatch m;
        const char s[] = "-ab,ab-";
        assert(std::regex_match(s, m, std::regex("-.*,.*-")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) >= 0 && static_cast<size_t>(m.length(0)) == std::char_traits<char>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::cmatch m;
        const char s[] = "a";
        assert(std::regex_match(s, m, std::regex("^[a]$")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) == 1);
        assert(m.position(0) == 0);
        assert(m.str(0) == "a");
    }
    {
        std::cmatch m;
        const char s[] = "a";
        assert(std::regex_match(s, m, std::regex("^[ab]$")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) == 1);
        assert(m.position(0) == 0);
        assert(m.str(0) == "a");
    }
    {
        std::cmatch m;
        const char s[] = "c";
        assert(std::regex_match(s, m, std::regex("^[a-f]$")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) == 1);
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::cmatch m;
        const char s[] = "g";
        assert(!std::regex_match(s, m, std::regex("^[a-f]$")));
        assert(m.size() == 0);
    }
    {
        std::cmatch m;
        const char s[] = "Iraqi";
        assert(!std::regex_match(s, m, std::regex("q[^u]")));
        assert(m.size() == 0);
    }
    {
        std::cmatch m;
        const char s[] = "Iraq";
        assert(!std::regex_match(s, m, std::regex("q[^u]")));
        assert(m.size() == 0);
    }
    {
        std::cmatch m;
        const char s[] = "AmB";
        assert(std::regex_match(s, m, std::regex("A[[:lower:]]B")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) >= 0 && static_cast<size_t>(m.length(0)) == std::char_traits<char>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::cmatch m;
        const char s[] = "AMB";
        assert(!std::regex_match(s, m, std::regex("A[[:lower:]]B")));
        assert(m.size() == 0);
    }
    {
        std::cmatch m;
        const char s[] = "AMB";
        assert(std::regex_match(s, m, std::regex("A[^[:lower:]]B")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) >= 0 && static_cast<size_t>(m.length(0)) == std::char_traits<char>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::cmatch m;
        const char s[] = "AmB";
        assert(!std::regex_match(s, m, std::regex("A[^[:lower:]]B")));
        assert(m.size() == 0);
    }
    {
        std::cmatch m;
        const char s[] = "A5B";
        assert(!std::regex_match(s, m, std::regex("A[^[:lower:]0-9]B")));
        assert(m.size() == 0);
    }
    {
        std::cmatch m;
        const char s[] = "A?B";
        assert(std::regex_match(s, m, std::regex("A[^[:lower:]0-9]B")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) >= 0 && static_cast<size_t>(m.length(0)) == std::char_traits<char>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::cmatch m;
        const char s[] = "-";
        assert(std::regex_match(s, m, std::regex("[a[.hyphen.]z]")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) >= 0 && static_cast<size_t>(m.length(0)) == std::char_traits<char>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::cmatch m;
        const char s[] = "z";
        assert(std::regex_match(s, m, std::regex("[a[.hyphen.]z]")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) >= 0 && static_cast<size_t>(m.length(0)) == std::char_traits<char>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::cmatch m;
        const char s[] = "m";
        assert(!std::regex_match(s, m, std::regex("[a[.hyphen.]z]")));
        assert(m.size() == 0);
    }
    std::locale::global(std::locale(LOCALE_cs_CZ_ISO8859_2));
    {
        std::cmatch m;
        const char s[] = "m";
        assert(std::regex_match(s, m, std::regex("[a[=M=]z]")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) >= 0 && static_cast<size_t>(m.length(0)) == std::char_traits<char>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::cmatch m;
        const char s[] = "Ch";
        assert(std::regex_match(s, m, std::regex("[a[.ch.]z]",
                   std::regex_constants::icase)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) >= 0 && static_cast<size_t>(m.length(0)) == std::char_traits<char>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::cmatch m;
        const char s[] = "foobar";
        assert(std::regex_match(s, m, std::regex("[^\\0]*")));
        assert(m.size() == 1);
    }
    {
        std::cmatch m;
        const char s[] = "foo\0bar";
        assert(std::regex_match(s, s+7, m, std::regex("[abfor\\0]*")));
        assert(m.size() == 1);
    }
    std::locale::global(std::locale("C"));
    {
        std::cmatch m;
        const char s[] = "m";
        assert(!std::regex_match(s, m, std::regex("[a[=M=]z]")));
        assert(m.size() == 0);
    }
    {
        std::cmatch m;
        const char s[] = "01a45cef9";
        assert(!std::regex_match(s, m, std::regex("[ace1-9]*")));
        assert(m.size() == 0);
    }
    {
        std::cmatch m;
        const char s[] = "01a45cef9";
        assert(!std::regex_match(s, m, std::regex("[ace1-9]+")));
        assert(m.size() == 0);
    }
    {
        const char r[] = "^[-+]?[0-9]+[CF]$";
        std::ptrdiff_t sr = std::char_traits<char>::length(r);
        typedef forward_iterator<const char*> FI;
        typedef bidirectional_iterator<const char*> BI;
        std::regex regex(FI(r), FI(r+sr));
        std::match_results<BI> m;
        const char s[] = "-40C";
        std::ptrdiff_t ss = std::char_traits<char>::length(s);
        assert(std::regex_match(BI(s), BI(s+ss), m, regex));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == BI(s));
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) == 4);
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::cmatch m;
        const char s[] = "Jeff Jeffs ";
        assert(!std::regex_match(s, m, std::regex("Jeff(?=s\\b)")));
        assert(m.size() == 0);
    }
    {
        std::cmatch m;
        const char s[] = "Jeffs Jeff";
        assert(!std::regex_match(s, m, std::regex("Jeff(?!s\\b)")));
        assert(m.size() == 0);
    }
    {
        std::cmatch m;
        const char s[] = "5%k";
        assert(std::regex_match(s, m, std::regex("\\d[\\W]k")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s + std::char_traits<char>::length(s));
        assert(m.length(0) >= 0 && static_cast<size_t>(m.length(0)) == std::char_traits<char>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }

    {
        std::wcmatch m;
        const wchar_t s[] = L"a";
        assert(std::regex_match(s, m, std::wregex(L"a")));
        assert(m.size() == 1);
        assert(!m.empty());
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s+1);
        assert(m.length(0) == 1);
        assert(m.position(0) == 0);
        assert(m.str(0) == L"a");
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"ab";
        assert(std::regex_match(s, m, std::wregex(L"ab")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s+2);
        assert(m.length(0) == 2);
        assert(m.position(0) == 0);
        assert(m.str(0) == L"ab");
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"ab";
        assert(!std::regex_match(s, m, std::wregex(L"ba")));
        assert(m.size() == 0);
        assert(m.empty());
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"aab";
        assert(!std::regex_match(s, m, std::wregex(L"ab")));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"aab";
        assert(!std::regex_match(s, m, std::wregex(L"ab"),
                                            std::regex_constants::match_continuous));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"abcd";
        assert(!std::regex_match(s, m, std::wregex(L"bc")));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"abbc";
        assert(std::regex_match(s, m, std::wregex(L"ab*c")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s+4);
        assert(m.length(0) == 4);
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"ababc";
        assert(std::regex_match(s, m, std::wregex(L"(ab)*c")));
        assert(m.size() == 2);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s+5);
        assert(m.length(0) == 5);
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
        assert(m.length(1) == 2);
        assert(m.position(1) == 2);
        assert(m.str(1) == L"ab");
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"abcdefghijk";
        assert(!std::regex_match(s, m, std::wregex(L"cd((e)fg)hi")));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"abc";
        assert(std::regex_match(s, m, std::wregex(L"^abc")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s+3);
        assert(m.length(0) == 3);
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"abcd";
        assert(!std::regex_match(s, m, std::wregex(L"^abc")));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"aabc";
        assert(!std::regex_match(s, m, std::wregex(L"^abc")));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"abc";
        assert(std::regex_match(s, m, std::wregex(L"abc$")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s+3);
        assert(m.length(0) == 3);
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"efabc";
        assert(!std::regex_match(s, m, std::wregex(L"abc$")));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"efabcg";
        assert(!std::regex_match(s, m, std::wregex(L"abc$")));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"abc";
        assert(std::regex_match(s, m, std::wregex(L"a.c")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s+3);
        assert(m.length(0) == 3);
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"acc";
        assert(std::regex_match(s, m, std::wregex(L"a.c")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s+3);
        assert(m.length(0) == 3);
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"acc";
        assert(std::regex_match(s, m, std::wregex(L"a.c")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s+3);
        assert(m.length(0) == 3);
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"abcdef";
        assert(std::regex_match(s, m, std::wregex(L"(.*).*")));
        assert(m.size() == 2);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s+6);
        assert(m.length(0) == 6);
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
        assert(m.length(1) == 6);
        assert(m.position(1) == 0);
        assert(m.str(1) == s);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"bc";
        assert(!std::regex_match(s, m, std::wregex(L"(a*)*")));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"abbc";
        assert(!std::regex_match(s, m, std::wregex(L"ab{3,5}c")));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"abbbc";
        assert(std::regex_match(s, m, std::wregex(L"ab{3,5}c")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) >= 0 && static_cast<size_t>(m.length(0)) == std::char_traits<wchar_t>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"abbbbc";
        assert(std::regex_match(s, m, std::wregex(L"ab{3,5}c")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) >= 0 && static_cast<size_t>(m.length(0)) == std::char_traits<wchar_t>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"abbbbbc";
        assert(std::regex_match(s, m, std::wregex(L"ab{3,5}c")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) >= 0 && static_cast<size_t>(m.length(0)) == std::char_traits<wchar_t>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"adefc";
        assert(!std::regex_match(s, m, std::wregex(L"ab{3,5}c")));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"abbbbbbc";
        assert(!std::regex_match(s, m, std::wregex(L"ab{3,5}c")));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"adec";
        assert(!std::regex_match(s, m, std::wregex(L"a.{3,5}c")));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"adefc";
        assert(std::regex_match(s, m, std::wregex(L"a.{3,5}c")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) >= 0 && static_cast<size_t>(m.length(0)) == std::char_traits<wchar_t>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"adefgc";
        assert(std::regex_match(s, m, std::wregex(L"a.{3,5}c")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) >= 0 && static_cast<size_t>(m.length(0)) == std::char_traits<wchar_t>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"adefghc";
        assert(std::regex_match(s, m, std::wregex(L"a.{3,5}c")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) >= 0 && static_cast<size_t>(m.length(0)) == std::char_traits<wchar_t>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"adefghic";
        assert(!std::regex_match(s, m, std::wregex(L"a.{3,5}c")));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        // http://www.open-std.org/jtc1/sc22/wg21/docs/lwg-defects.html#2273
        const wchar_t s[] = L"tournament";
        assert(std::regex_match(s, m, std::wregex(L"tour|to|tournament")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) >= 0 && static_cast<size_t>(m.length(0)) == std::char_traits<wchar_t>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::wcmatch m;
        // http://www.open-std.org/jtc1/sc22/wg21/docs/lwg-defects.html#2273
        const wchar_t s[] = L"tournamenttotour";
        assert(
            std::regex_match(s, m, std::wregex(L"(tour|to|tournament)+",
                                               std::regex_constants::nosubs)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) >= 0 && static_cast<size_t>(m.length(0)) == std::char_traits<wchar_t>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"ttotour";
        assert(std::regex_match(s, m, std::wregex(L"(tour|to|t)+")));
        assert(m.size() == 2);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) >= 0 && static_cast<size_t>(m.length(0)) == std::char_traits<wchar_t>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
        assert(m.length(1) == 4);
        assert(m.position(1) == 3);
        assert(m.str(1) == L"tour");
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"-ab,ab-";
        assert(!std::regex_match(s, m, std::wregex(L"-(.*),\1-")));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"-ab,ab-";
        assert(std::regex_match(s, m, std::wregex(L"-.*,.*-")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) >= 0 && static_cast<size_t>(m.length(0)) == std::char_traits<wchar_t>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"a";
        assert(std::regex_match(s, m, std::wregex(L"^[a]$")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) == 1);
        assert(m.position(0) == 0);
        assert(m.str(0) == L"a");
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"a";
        assert(std::regex_match(s, m, std::wregex(L"^[ab]$")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) == 1);
        assert(m.position(0) == 0);
        assert(m.str(0) == L"a");
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"c";
        assert(std::regex_match(s, m, std::wregex(L"^[a-f]$")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) == 1);
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"g";
        assert(!std::regex_match(s, m, std::wregex(L"^[a-f]$")));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"Iraqi";
        assert(!std::regex_match(s, m, std::wregex(L"q[^u]")));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"Iraq";
        assert(!std::regex_match(s, m, std::wregex(L"q[^u]")));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"AmB";
        assert(std::regex_match(s, m, std::wregex(L"A[[:lower:]]B")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) >= 0 && static_cast<size_t>(m.length(0)) == std::char_traits<wchar_t>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"AMB";
        assert(!std::regex_match(s, m, std::wregex(L"A[[:lower:]]B")));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"AMB";
        assert(std::regex_match(s, m, std::wregex(L"A[^[:lower:]]B")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) >= 0 && static_cast<size_t>(m.length(0)) == std::char_traits<wchar_t>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"AmB";
        assert(!std::regex_match(s, m, std::wregex(L"A[^[:lower:]]B")));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"A5B";
        assert(!std::regex_match(s, m, std::wregex(L"A[^[:lower:]0-9]B")));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"A?B";
        assert(std::regex_match(s, m, std::wregex(L"A[^[:lower:]0-9]B")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) >= 0 && static_cast<size_t>(m.length(0)) == std::char_traits<wchar_t>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"-";
        assert(std::regex_match(s, m, std::wregex(L"[a[.hyphen.]z]")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) >= 0 && static_cast<size_t>(m.length(0)) == std::char_traits<wchar_t>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"z";
        assert(std::regex_match(s, m, std::wregex(L"[a[.hyphen.]z]")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) >= 0 && static_cast<size_t>(m.length(0)) == std::char_traits<wchar_t>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"m";
        assert(!std::regex_match(s, m, std::wregex(L"[a[.hyphen.]z]")));
        assert(m.size() == 0);
    }
    std::locale::global(std::locale(LOCALE_cs_CZ_ISO8859_2));
    {
        std::wcmatch m;
        const wchar_t s[] = L"m";
        assert(std::regex_match(s, m, std::wregex(L"[a[=M=]z]")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) >= 0 && static_cast<size_t>(m.length(0)) == std::char_traits<wchar_t>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"Ch";
        assert(std::regex_match(s, m, std::wregex(L"[a[.ch.]z]",
                   std::regex_constants::icase)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) >= 0 && static_cast<size_t>(m.length(0)) == std::char_traits<wchar_t>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    std::locale::global(std::locale("C"));
    {
        std::wcmatch m;
        const wchar_t s[] = L"m";
        assert(!std::regex_match(s, m, std::wregex(L"[a[=M=]z]")));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"01a45cef9";
        assert(!std::regex_match(s, m, std::wregex(L"[ace1-9]*")));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"01a45cef9";
        assert(!std::regex_match(s, m, std::wregex(L"[ace1-9]+")));
        assert(m.size() == 0);
    }
    {
        const wchar_t r[] = L"^[-+]?[0-9]+[CF]$";
        std::ptrdiff_t sr = std::char_traits<wchar_t>::length(r);
        typedef forward_iterator<const wchar_t*> FI;
        typedef bidirectional_iterator<const wchar_t*> BI;
        std::wregex regex(FI(r), FI(r+sr));
        std::match_results<BI> m;
        const wchar_t s[] = L"-40C";
        std::ptrdiff_t ss = std::char_traits<wchar_t>::length(s);
        assert(std::regex_match(BI(s), BI(s+ss), m, regex));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == BI(s));
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == m[0].second);
        assert(m.length(0) == 4);
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"Jeff Jeffs ";
        assert(!std::regex_match(s, m, std::wregex(L"Jeff(?=s\\b)")));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"Jeffs Jeff";
        assert(!std::regex_match(s, m, std::wregex(L"Jeff(?!s\\b)")));
        assert(m.size() == 0);
    }
    {
        std::wcmatch m;
        const wchar_t s[] = L"5%k";
        assert(std::regex_match(s, m, std::wregex(L"\\d[\\W]k")));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s + std::char_traits<wchar_t>::length(s));
        assert(m.length(0) >= 0 && static_cast<size_t>(m.length(0)) == std::char_traits<wchar_t>::length(s));
        assert(m.position(0) == 0);
        assert(m.str(0) == s);
    }
}
