
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// <iterator> feature macros

/*  Constant                                    Value
    __cpp_lib_array_constexpr                   201603L
    __cpp_lib_make_reverse_iterator             201402L
    __cpp_lib_nonmember_container_access        201411L
    __cpp_lib_null_iterators                    201304L

*/

#include <iterator>
#include <cassert>
#include "test_macros.h"

int main()
{
//  ensure that the macros that are supposed to be defined in <iterator> are defined.

/*
#if !defined(__cpp_lib_fooby)
# error "__cpp_lib_fooby is not defined"
#elif __cpp_lib_fooby < 201606L
# error "__cpp_lib_fooby has an invalid value"
#endif
*/
}
