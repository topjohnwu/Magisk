//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// conditional

#include <type_traits>

#include "test_macros.h"

int main()
{
    static_assert((std::is_same<std::conditional<true, char, int>::type, char>::value), "");
    static_assert((std::is_same<std::conditional<false, char, int>::type, int>::value), "");
#if TEST_STD_VER > 11
    static_assert((std::is_same<std::conditional_t<true, char, int>, char>::value), "");
    static_assert((std::is_same<std::conditional_t<false, char, int>, int>::value), "");
#endif
}
