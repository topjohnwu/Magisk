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
// basic_ostream<charT, traits>&
// operator<<(basic_ostream<charT, traits>& os, const bitset<N>& x);

#include <bitset>
#include <sstream>
#include <cassert>

int main()
{
    std::istringstream in("01011010");
    std::bitset<8> b;
    in >> b;
    assert(b.to_ulong() == 0x5A);
}
