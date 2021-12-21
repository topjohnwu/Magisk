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

// static const iostate badbit;
// static const iostate eofbit;
// static const iostate failbit;
// static const iostate goodbit = 0;

#include <ios>
#include <cassert>

int main()
{
    assert(std::ios_base::badbit);
    assert(std::ios_base::eofbit);
    assert(std::ios_base::failbit);

    assert
    (
        ( std::ios_base::badbit
        & std::ios_base::eofbit
        & std::ios_base::failbit) == 0
    );

    assert(std::ios_base::goodbit == 0);
}
