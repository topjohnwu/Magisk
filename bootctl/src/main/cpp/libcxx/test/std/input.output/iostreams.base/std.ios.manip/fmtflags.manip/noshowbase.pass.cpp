//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <ios>

// class ios_base

// ios_base& noshowbase(ios_base& str);

#include <ios>
#include <streambuf>
#include <cassert>

struct testbuf : public std::streambuf {};

int main()
{
    testbuf sb;
    std::ios ios(&sb);
    std::showbase(ios);
    std::ios_base& r = std::noshowbase(ios);
    assert(&r == &ios);
    assert(!(ios.flags() & std::ios::showbase));
}
