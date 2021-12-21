//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <regex>

// template <class BidirectionalIterator, class Allocator, class charT, class traits>
//     bool
//     regex_match(BidirectionalIterator first, BidirectionalIterator last,
//                  match_results<BidirectionalIterator, Allocator>& m,
//                  const basic_regex<charT, traits>& e,
//                  regex_constants::match_flag_type flags = regex_constants::match_default);

// std::regex in ECMAScript mode should not ignore capture groups inside lookahead assertions.
// For example, matching /(?=(a))(a)/ to "a" should yield two captures: \1 = "a", \2 = "a"

#include <regex>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"

int main()
{
    {
        std::regex re("^(?=(.))a$");
        assert(re.mark_count() == 1);

        std::string s("a");
        std::smatch m;
        assert(std::regex_match(s, m, re));
        assert(m.size() == 2);
        assert(m[0] == "a");
        assert(m[1] == "a");
    }

    {
        std::regex re("^(a)(?=(.))(b)$");
        assert(re.mark_count() == 3);

        std::string s("ab");
        std::smatch m;
        assert(std::regex_match(s, m, re));
        assert(m.size() == 4);
        assert(m[0] == "ab");
        assert(m[1] == "a");
        assert(m[2] == "b");
        assert(m[3] == "b");
    }

    {
        std::regex re("^(.)(?=(.)(?=.(.)))(...)$");
        assert(re.mark_count() == 4);

        std::string s("abcd");
        std::smatch m;
        assert(std::regex_match(s, m, re));
        assert(m.size() == 5);
        assert(m[0] == "abcd");
        assert(m[1] == "a");
        assert(m[2] == "b");
        assert(m[3] == "d");
        assert(m[4] == "bcd");
    }

    {
        std::regex re("^(a)(?!([^b]))(.c)$");
        assert(re.mark_count() == 3);

        std::string s("abc");
        std::smatch m;
        assert(std::regex_match(s, m, re));
        assert(m.size() == 4);
        assert(m[0] == "abc");
        assert(m[1] == "a");
        assert(m[2] == "");
        assert(m[3] == "bc");
    }

    {
        std::regex re("^(?!((b)))(?=(.))(?!(abc)).b$");
        assert(re.mark_count() == 4);

        std::string s("ab");
        std::smatch m;
        assert(std::regex_match(s, m, re));
        assert(m.size() == 5);
        assert(m[0] == "ab");
        assert(m[1] == "");
        assert(m[2] == "");
        assert(m[3] == "a");
        assert(m[4] == "");
    }
}
