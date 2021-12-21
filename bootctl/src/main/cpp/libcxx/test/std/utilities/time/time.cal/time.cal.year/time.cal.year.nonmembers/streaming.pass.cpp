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
// class year;

// template<class charT, class traits>
//   basic_ostream<charT, traits>&
//   operator<<(basic_ostream<charT, traits>& os, const year& y);
//
//   Effects: Inserts format(fmt, y) where fmt is "%Y" widened to charT.
//   If !y.ok(), appends with " is not a valid year".
//
// template<class charT, class traits>
//   basic_ostream<charT, traits>&
//   to_stream(basic_ostream<charT, traits>& os, const charT* fmt, const year& y);
//
//   Effects: Streams y into os using the format specified by the NTCTS fmt.
//     fmt encoding follows the rules specified in 25.11.
//
// template<class charT, class traits, class Alloc = allocator<charT>>
//   basic_istream<charT, traits>&
//   from_stream(basic_istream<charT, traits>& is, const charT* fmt,
//               year& y, basic_string<charT, traits, Alloc>* abbrev = nullptr,
//               minutes* offset = nullptr);
//
//   Effects: Attempts to parse the input stream is into the year y using the format flags
//     given in the NTCTS fmt as specified in 25.12. If the parse fails to decode a valid year,
//     is.setstate(ios_base::failbit) shall be called and y shall not be modified. If %Z is used
//     and successfully parsed, that value will be assigned to *abbrev if abbrev is non-null.
//     If %z (or a modified variant) is used and successfully parsed, that value will be
//     assigned to *offset if offset is non-null.


#include <chrono>
#include <type_traits>
#include <cassert>
#include <iostream>

#include "test_macros.h"

int main()
{
   using year = std::chrono::year;

   std::cout << year{2018};
}
