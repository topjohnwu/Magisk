//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <ios>

// template <class charT, class traits> class basic_ios

// bool bad() const;

#include <ios>
#include <streambuf>
#include <cassert>

struct testbuf : public std::streambuf {};

int main()
{
    {
        std::ios ios(0);
        assert(ios.bad());
        ios.setstate(std::ios::eofbit);
        assert(ios.bad());
    }
    {
        testbuf sb;
        std::ios ios(&sb);
        assert(!ios.bad());
        ios.setstate(std::ios::eofbit);
        assert(!ios.bad());
        ios.setstate(std::ios::failbit);
        assert(!ios.bad());
        ios.setstate(std::ios::badbit);
        assert(ios.bad());
    }
}
