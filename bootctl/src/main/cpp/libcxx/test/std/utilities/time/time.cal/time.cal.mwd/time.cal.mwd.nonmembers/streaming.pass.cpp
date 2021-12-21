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
// class month_weekday;

// template<class charT, class traits>
//     basic_ostream<charT, traits>&
//     operator<<(basic_ostream<charT, traits>& os, const month_weekday& mwd);
//
//     Returns: os << mwd.month() << '/' << mwd.weekday_indexed().

#include <chrono>
#include <type_traits>
#include <cassert>
#include <iostream>

#include "test_macros.h"

int main()
{
    using month_weekday   = std::chrono::month_weekday;
    using month           = std::chrono::month;
    using weekday_indexed = std::chrono::weekday_indexed;
    using weekday         = std::chrono::weekday;

    std::cout << month_weekday{month{1}, weekday_indexed{weekday{3}, 3}};
}
