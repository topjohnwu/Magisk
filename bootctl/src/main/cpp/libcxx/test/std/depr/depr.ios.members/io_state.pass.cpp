//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <ios>
//
// class ios_base
// {
// public:
//     typedef T1 io_state;
// };

//  These members were removed for C++17

#include "test_macros.h"
#include <strstream>
#include <cassert>

int main()
{
#if TEST_STD_VER <= 14
    std::strstream::io_state b = std::strstream::eofbit;
    assert(b == std::ios::eofbit);
#endif
}
