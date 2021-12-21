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

// fmtflags flags(fmtflags fmtfl);

#include <ios>
#include <cassert>

class test
    : public std::ios
{
public:
    test()
    {
        init(0);
    }
};

int main()
{
    test t;
    assert(t.flags() == (test::skipws | test::dec));
    test::fmtflags f = t.flags(test::hex | test::right);
    assert(f == (test::skipws | test::dec));
    assert(t.flags() == (test::hex | test::right));
}
