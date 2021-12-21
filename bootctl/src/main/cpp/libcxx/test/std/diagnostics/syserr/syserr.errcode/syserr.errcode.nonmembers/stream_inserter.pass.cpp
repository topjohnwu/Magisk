//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <system_error>

// class error_code

// template <class charT, class traits>
//   basic_ostream<charT,traits>&
//   operator<<(basic_ostream<charT,traits>& os, const error_code& ec);

#include <system_error>
#include <sstream>
#include <cassert>

int main()
{
    std::ostringstream out;
    out << std::error_code(std::io_errc::stream);
    assert(out.str() == "iostream:1");
}
