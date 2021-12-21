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
// class month_day;

// template<class charT, class traits>
//     basic_ostream<charT, traits>&
//     operator<<(basic_ostream<charT, traits>& os, const month_day& md);
//
//     Returns: os << md.month() << '/' << md.day().
//
// template<class charT, class traits>
//     basic_ostream<charT, traits>&
//     to_stream(basic_ostream<charT, traits>& os, const charT* fmt, const month_day& md);
//
// Effects: Streams md into os using the format specified by the NTCTS fmt.
//          fmt encoding follows the rules specified in 25.11.


#include <chrono>
#include <type_traits>
#include <cassert>
#include <iostream>
#include "test_macros.h"

int main()
{
    using month_day = std::chrono::month_day;
    using month     = std::chrono::month;
    using day       = std::chrono::day;
    std::cout << month_day{month{1}, day{1}};
}
