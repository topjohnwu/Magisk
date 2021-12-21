//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test:

// template <class charT, class traits, size_t N>
// basic_istream<charT, traits>&
// operator>>(basic_istream<charT, traits>& is, bitset<N>& x);

#include <bitset>
#include <sstream>
#include <cassert>

int main()
{
    std::ostringstream os;
    std::bitset<8> b(0x5A);
    os << b;
    assert(os.str() == "01011010");
}
