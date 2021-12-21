//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// UNSUPPORTED: c++98, c++03, c++11, c++14

//	We have two macros for checking whether or not the underlying C library
//	 has C11 features:
//		TEST_HAS_C11_FEATURES    - which is defined in "test_macros.h"
//		_LIBCPP_HAS_C11_FEATURES - which is defined in <__config>
//	They should always be the same

#include <__config>
#include "test_macros.h"

#ifdef TEST_HAS_C11_FEATURES
# ifndef _LIBCPP_HAS_C11_FEATURES
#  error "TEST_HAS_C11_FEATURES is defined, but _LIBCPP_HAS_C11_FEATURES is not"
# endif
#endif

#ifdef _LIBCPP_HAS_C11_FEATURES
# ifndef TEST_HAS_C11_FEATURES
#  error "_LIBCPP_HAS_C11_FEATURES is defined, but TEST_HAS_C11_FEATURES is not"
# endif
#endif

int main() {}
