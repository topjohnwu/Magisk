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

// long& iword(int idx);

// This test compiles but never completes when compiled against the MSVC STL
// UNSUPPORTED: msvc

#include <ios>
#include <string>
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
    std::ios_base& b = t;
    for (int i = 0; i < 10000; ++i)
    {
        assert(b.iword(i) == 0);
        b.iword(i) = i;
        assert(b.iword(i) == i);
        for (int j = 0; j <= i; ++j)
            assert(b.iword(j) == j);
    }
}
