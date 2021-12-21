// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Test that _LIBCPP_NODISCARD_AFTER_CXX17 is disabled whenever
// _LIBCPP_DISABLE_NODISCARD_AFTER_CXX17 is defined by the user.

// MODULES_DEFINES: _LIBCPP_DISABLE_NODISCARD_AFTER_CXX17
#define _LIBCPP_DISABLE_NODISCARD_AFTER_CXX17
#include <__config>

_LIBCPP_NODISCARD_AFTER_CXX17 int foo() { return 6; }

int main ()
{
	foo();	// no error here!
}
