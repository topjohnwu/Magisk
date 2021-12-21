//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: libcpp-no-exceptions

// "support/test_macros.hpp"

// #define TEST_HAS_NO_EXCEPTIONS

#include "test_macros.h"

#if defined(TEST_HAS_NO_EXCEPTIONS)
#error macro defined unexpectedly
#endif

int main() {
    try { ((void)0); } catch (...) {}
}
