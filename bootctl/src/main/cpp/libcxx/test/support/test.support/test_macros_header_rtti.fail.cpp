//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// "support/test_macros.hpp"

// #define TEST_HAS_NO_RTTI

#include "test_macros.h"

struct A { virtual ~A() {} };
struct B : A {};

int main() {
#if defined(TEST_HAS_NO_RTTI)
    A* ptr = new B;
    (void)dynamic_cast<B*>(ptr); // expected-error{{cannot use dynamic_cast}}
#else
    A* ptr = new B;
    (void)dynamic_cast<B*>(ptr);
#error RTTI enabled
// expected-error@-1{{RTTI enabled}}
#endif
}
