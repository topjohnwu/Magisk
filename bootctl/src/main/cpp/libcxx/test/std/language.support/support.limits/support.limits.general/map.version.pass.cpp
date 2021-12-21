
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// <map> feature macros

/*  Constant                                    Value
    __cpp_lib_allocator_traits_is_always_equal  201411L
    __cpp_lib_erase_if                          201811L
    __cpp_lib_generic_associative_lookup        201304L
    __cpp_lib_map_try_emplace                   201411L
    __cpp_lib_node_extract                      201606L
    __cpp_lib_nonmember_container_access        201411L

*/

#include <map>
#include <cassert>
#include "test_macros.h"

int main()
{
//  ensure that the macros that are supposed to be defined in <map> are defined.

#if TEST_STD_VER > 17
# if !defined(__cpp_lib_erase_if)  
  LIBCPP_STATIC_ASSERT(false, "__cpp_lib_erase_if is not defined");
# else
#  if __cpp_lib_erase_if < 201811L
#   error "__cpp_lib_erase_if has an invalid value"
#  endif
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
