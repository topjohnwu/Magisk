//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <regex>

// template <class traits, class charT, class ST, class SA, class FST, class FSA>>
//     basic_string<charT, ST, SA>
//     regex_replace(const basic_string<charT, ST, SA>& s,
//                   const basic_regex<charT, traits>& e,
//                   const basic_string<charT, FST, FSA>& fmt,
//                   regex_constants::match_flag_type flags =
//                                              regex_constants::match_default);

#include <regex>
#include <cassert>
#include "test_macros.h"

int main()
{
    {
        std::regex phone_numbers("\\d{3}-\\d{4}");
        std::string phone_book("555-1234, 555-2345, 555-3456");
        std::string r = std::regex_replace(phone_book, phone_numbers,
                                           std::string("123-$&"));
        assert(r == "123-555-1234, 123-555-2345, 123-555-3456");
    }
    {
        std::regex phone_numbers("\\d{3}-\\d{4}");
        std::string phone_book("555-1234, 555-2345, 555-3456");
        std::string r = std::regex_replace(phone_book, phone_numbers,
                                           std::string("123-$&"),
                                           std::regex_constants::format_sed);
        assert(r == "123-$555-1234, 123-$555-2345, 123-$555-3456");
    }
    {
        std::regex phone_numbers("\\d{3}-\\d{4}");
        std::string phone_book("555-1234, 555-2345, 555-3456");
        std::string r = std::regex_replace(phone_book, phone_numbers,
                                           std::string("123-&"),
                                           std::regex_constants::format_sed);
        assert(r == "123-555-1234, 123-555-2345, 123-555-3456");
    }
    {
        std::regex phone_numbers("\\d{3}-\\d{4}");
        std::string phone_book("555-1234, 555-2345, 555-3456");
        std::string r = std::regex_replace(phone_book, phone_numbers,
                                           std::string("123-$&"),
                                           std::regex_constants::format_no_copy);
        assert(r == "123-555-1234123-555-2345123-555-3456");
    }
    {
        std::regex phone_numbers("\\d{3}-\\d{4}");
        std::string phone_book("555-1234, 555-2345, 555-3456");
        std::string r = std::regex_replace(phone_book, phone_numbers,
                                           std::string("123-$&"),
                                           std::regex_constants::format_first_only);
        assert(r == "123-555-1234, 555-2345, 555-3456");
    }
    {
        std::regex phone_numbers("\\d{3}-\\d{4}");
        std::string phone_book("555-1234, 555-2345, 555-3456");
        std::string r = std::regex_replace(phone_book, phone_numbers,
                                           std::string("123-$&"),
                                           std::regex_constants::format_first_only |
                                           std::regex_constants::format_no_copy);
        assert(r == "123-555-1234");
    }
}
