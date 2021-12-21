
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// <atomic> feature macros

/*  Constant                                    Value
    __cpp_lib_char8_t                           201811L
    __cpp_lib_atomic_is_always_lock_free        201603L
    __cpp_lib_atomic_ref                        201806L

*/

// UNSUPPORTED: libcpp-has-no-threads

#include <atomic>
#include <cassert>
#include "test_macros.h"

int main()
{
//  ensure that the macros that are supposed to be defined in <atomic> are defined.

#if TEST_STD_VER > 17 && defined(__cpp_char8_t)
# if !defined(__cpp_lib_char8_t)  
  LIBCPP_STATIC_ASSERT(false, "__cpp_lib_char8_t is not defined");
# else
#  if __cpp_lib_char8_t < 201811L
#   error "__cpp_lib_char8_t has an invalid value"
#  endif
# endif
#endif

#if TEST_STD_VER > 14
# if !defined(__cpp_lib_atomic_is_always_lock_free)
#  error "__cpp_lib_atomic_is_always_lock_free is not defined"
# elif __cpp_lib_atomic_is_always_lock_free < 201603L
#  error "__cpp_lib_atomic_is_always_lock_free has an invalid value"
# endif
#endif

/*
#if !defined(__cpp_lib_fooby)
# error "__cpp_lib_fooby is not defined"
#elif __cpp_lib_fooby < 201606L
# error "__cpp_lib_fooby has an invalid value"
#endif
*/
}
