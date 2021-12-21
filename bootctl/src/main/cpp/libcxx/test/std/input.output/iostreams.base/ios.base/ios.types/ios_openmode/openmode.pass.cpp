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

// static const openmode app;
// static const openmode ate;
// static const openmode binary;
// static const openmode in;
// static const openmode out;
// static const openmode trunc;

#include <ios>
#include <cassert>

int main()
{
    assert(std::ios_base::app);
    assert(std::ios_base::ate);
    assert(std::ios_base::binary);
    assert(std::ios_base::in);
    assert(std::ios_base::out);
    assert(std::ios_base::trunc);

    assert
    (
        ( std::ios_base::app
        & std::ios_base::ate
        & std::ios_base::binary
        & std::ios_base::in
        & std::ios_base::out
        & std::ios_base::trunc) == 0
    );
}
