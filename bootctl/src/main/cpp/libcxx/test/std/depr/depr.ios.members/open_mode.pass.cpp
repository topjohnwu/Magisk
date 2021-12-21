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
//     typedef T2 open_mode;
// };

//  These members were removed for C++17

#include "test_macros.h"
#include <strstream>
#include <cassert>

int main()
{
#if TEST_STD_VER <= 14
    std::strstream::open_mode b = std::strstream::app;
    assert(b == std::ios::app);
#endif
}
