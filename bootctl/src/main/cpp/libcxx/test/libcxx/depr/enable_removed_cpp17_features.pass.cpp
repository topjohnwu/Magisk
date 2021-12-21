//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Test that defining _LIBCPP_ENABLE_CXX17_REMOVED_FEATURES correctly defines
// _LIBCPP_ENABLE_CXX17_REMOVED_FOO for each individual component macro.

// MODULES_DEFINES: _LIBCPP_ENABLE_CXX17_REMOVED_FEATURES
#define _LIBCPP_ENABLE_CXX17_REMOVED_FEATURES
#include <__config>

#ifndef _LIBCPP_ENABLE_CXX17_REMOVED_UNEXPECTED_FUNCTIONS
#error _LIBCPP_ENABLE_CXX17_REMOVED_UNEXPECTED_FUNCTIONS must be defined
#endif

#ifndef _LIBCPP_ENABLE_CXX17_REMOVED_AUTO_PTR
#error _LIBCPP_ENABLE_CXX17_REMOVED_AUTO_PTR must be defined
#endif

int main() {
}
