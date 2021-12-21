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
//     typedef POS_T streampos;
// };

//  These members were removed for C++17

#include "test_macros.h"
#include <ios>
#include <type_traits>

int main()
{
#if TEST_STD_VER <= 14
    static_assert((std::is_same<std::ios_base::streampos, std::streampos>::value), "");
#endif
}
