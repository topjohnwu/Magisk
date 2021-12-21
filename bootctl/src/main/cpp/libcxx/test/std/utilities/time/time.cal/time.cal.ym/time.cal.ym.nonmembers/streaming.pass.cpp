//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// UNSUPPORTED: c++98, c++03, c++11, c++14, c++17
// XFAIL: *

// <chrono>
// class year_month;

// template<class charT, class traits>
//     basic_ostream<charT, traits>&
//     operator<<(basic_ostream<charT, traits>& os, const year_month& ym);
//
// Returns: os << ym.year() << '/' << ym.month().
//
//
// template<class charT, class traits>
//     basic_ostream<charT, traits>&
//     to_stream(basic_ostream<charT, traits>& os, const charT* fmt, const year_month& ym);
//
// Effects: Streams ym into os using the format specified by the NTCTS fmt. fmt encoding follows the rules specified in 25.11.
//
// template<class charT, class traits, class Alloc = allocator<charT>>
//     basic_istream<charT, traits>&
//   from_stream(basic_istream<charT, traits>& is, const charT* fmt,
//               year_month& ym, basic_string<charT, traits, Alloc>* abbrev = nullptr,
//               minutes* offset = nullptr);
//
// Effects: Attempts to parse the input stream is into the year_month ym using the format
//         flags given in the NTCTS fmt as specified in 25.12. If the parse fails to decode
//         a valid year_month, is.setstate(ios_- base::failbit) shall be called and ym shall
//         not be modified. If %Z is used and successfully parsed, that value will be assigned
//         to *abbrev if abbrev is non-null. If %z (or a modified variant) is used and
//         successfully parsed, that value will be assigned to *offset if offset is non-null.



#include <chrono>
#include <type_traits>
#include <cassert>
#include <iostream>

#include "test_macros.h"

int main()
{
    using year_month = std::chrono::year_month;
    using year       = std::chrono::year;
    using month      = std::chrono::month;

    std::cout << year_month{year{2018}, month{3}};
}
