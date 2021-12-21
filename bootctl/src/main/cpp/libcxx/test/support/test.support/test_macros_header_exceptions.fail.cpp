//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// "support/test_macros.hpp"

// #define TEST_HAS_NO_EXCEPTIONS

#include "test_macros.h"

int main() {
#if defined(TEST_HAS_NO_EXCEPTIONS)
    try { ((void)0); } catch (...) {} // expected-error {{exceptions disabled}}
#else
    try { ((void)0); } catch (...) {}
#error exceptions enabled
// expected-error@-1 {{exceptions enabled}}
#endif
}
