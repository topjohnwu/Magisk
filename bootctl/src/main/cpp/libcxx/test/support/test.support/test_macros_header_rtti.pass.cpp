//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: libcpp-no-rtti

// "support/test_macros.hpp"

// #define TEST_HAS_NO_RTTI

#include "test_macros.h"

#if defined(TEST_HAS_NO_RTTI)
#error Macro defined unexpectedly
#endif

struct A { virtual ~A() {} };
struct B : A {};

int main() {
    A* ptr = new B;
    (void)dynamic_cast<B*>(ptr);
    delete ptr;
}
