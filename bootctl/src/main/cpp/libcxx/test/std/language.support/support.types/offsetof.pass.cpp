//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

#include <cstddef>

#include "test_macros.h"

#ifndef offsetof
#error offsetof not defined
#endif

struct A
{
    int x;
};

int main()
{
    static_assert(noexcept(offsetof(A, x)), "");
}
