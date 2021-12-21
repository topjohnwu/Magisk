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
// class month;

// template<class charT, class traits>
//   basic_ostream<charT, traits>&
//   operator<<(basic_ostream<charT, traits>& os, const month& m);
//
//   Effects: If m.ok() == true inserts format(os.getloc(), fmt, m) where fmt is "%b" widened to charT.
//   Otherwise inserts int{m} << " is not a valid month".
//
// template<class charT, class traits>
//   basic_ostream<charT, traits>&
//   to_stream(basic_ostream<charT, traits>& os, const charT* fmt, const month& m);
//
//   Effects: Streams m into os using the format specified by the NTCTS fmt.
//   fmt encoding follows the rules specified in 25.11.
//
// template<class charT, class traits, class Alloc = allocator<charT>>
//   basic_istream<charT, traits>&
//   from_stream(basic_istream<charT, traits>& is, const charT* fmt,
//             month& m, basic_string<charT, traits, Alloc>* abbrev = nullptr,
//             minutes* offset = nullptr);
//
//   Effects: Attempts to parse the input stream is into the month m using the format flags
//   given in the NTCTS fmt as specified in 25.12. If the parse fails to decode a valid month,
//   is.setstate(ios_- base::failbit) shall be called and m shall not be modified.
//   If %Z is used and successfully parsed, that value will be assigned to *abbrev if
//   abbrev is non-null. If %z (or a modified variant) is used and successfully parsed,
//   that value will be assigned to *offset if offset is non-null.

#include <chrono>
#include <type_traits>
#include <cassert>
#include <iostream>

#include "test_macros.h"

int main()
{
   using month = std::chrono::month;
   std::cout << month{1};
}
