//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// typedef minstd_rand0 default_random_engine;

#include <random>
#include <cassert>

#include "test_macros.h"

int main()
{
    std::default_random_engine e;
    e.discard(9999);
    LIBCPP_ASSERT(e() == 399268537u);
}
