//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <regex>

// class regex_iterator<BidirectionalIterator, charT, traits>

// template <std::size_t N>
// regex_token_iterator(BidirectionalIterator a, BidirectionalIterator b,
//                      const regex_type&& re,
//                      const std::vector<int>& submatches,
//                      regex_constants::match_flag_type m =
//                                              regex_constants::match_default);

#include <regex>
#include <cassert>
#include "test_macros.h"

#if TEST_STD_VER < 14
#error
#endif

int main()
{
    {
         std::regex phone_numbers("\\d{3}-(\\d{4})");
        const char phone_book[] = "start 555-1234, 555-2345, 555-3456 end";
        std::vector<int> v;
        v.push_back(-1);
        v.push_back(-1);
        std::cregex_token_iterator i(std::begin(phone_book), std::end(phone_book)-1,
                                     std::regex("\\d{3}-\\d{4}"), v);
    }
}
