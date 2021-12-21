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

// void*& pword(int idx);

// This test compiles but never completes when compiled against the MSVC STL
// UNSUPPORTED: msvc

#include <ios>
#include <string>
#include <cassert>
#include <cstdint>

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
    for (std::intptr_t i = 0; i < 10000; ++i)
    {
        assert(b.pword(i) == 0);
        b.pword(i) = (void*)i;
        assert(b.pword(i) == (void*)i);
        for (std::intptr_t j = 0; j <= i; ++j)
            assert(b.pword(j) == (void*)j);
    }
}
