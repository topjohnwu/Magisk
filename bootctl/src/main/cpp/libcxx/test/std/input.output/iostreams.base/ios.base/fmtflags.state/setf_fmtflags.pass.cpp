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

// fmtflags setf(fmtflags fmtfl)

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
    test::fmtflags f = t.setf(test::hex | test::right);
    assert(f == (test::skipws | test::dec));
    assert(t.flags() == (test::skipws | test::dec | test::hex | test::right));
}
