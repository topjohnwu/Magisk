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
//     regex_search(BidirectionalIterator first, BidirectionalIterator last,
//                  match_results<BidirectionalIterator, Allocator>& m,
//                  const basic_regex<charT, traits>& e,
//                  regex_constants::match_flag_type flags = regex_constants::match_default);

#include <regex>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"

extern "C" void LLVMFuzzerTestOneInput(const char *data)
{
#ifndef TEST_HAS_NO_EXCEPTIONS
    size_t size = strlen(data);
    if (size > 0)
    {
        try
        {
            std::regex::flag_type flag = std::regex_constants::grep;
            std::string s((const char *)data, size);
            std::regex re(s, flag);
            TEST_IGNORE_NODISCARD std::regex_match(s, re);
        }
        catch (std::regex_error &) {}
    }
#else
    ((void)data);
#endif
}


void fuzz_tests()  // patterns that the fuzzer has found
{
// Raw string literals are a C++11 feature.
#if TEST_STD_VER >= 11
    LLVMFuzzerTestOneInput(R"XX(Õ)_%()()((\8'_%()_%()_%()_%(()_%()_%()_%(.t;)()¥f()_%()(.)_%;)()!¥f(((()()XX");
#endif
}

int main()
{
    {
        std::cmatch m;
        const char s[] = "tournament";
        assert(std::regex_search(s, m, std::regex("tour\nto\ntournament",
                std::regex_constants::grep)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(!m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s + std::char_traits<char>::length(s));
        assert(m.length(0) == 10);
        assert(m.position(0) == 0);
        assert(m.str(0) == "tournament");
    }
    {
        std::cmatch m;
        const char s[] = "ment";
        assert(std::regex_search(s, m, std::regex("tour\n\ntournament",
                std::regex_constants::grep)));
        assert(m.size() == 1);
        assert(!m.prefix().matched);
        assert(m.prefix().first == s);
        assert(m.prefix().second == m[0].first);
        assert(m.suffix().matched);
        assert(m.suffix().first == m[0].second);
        assert(m.suffix().second == s + std::char_traits<char>::length(s));
        assert(m.length(0) == 0);
        assert(m.position(0) == 0);
        assert(m.str(0) == "");
    }
    fuzz_tests();
}
