
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// <tuple> feature macros

/*  Constant                                    Value
    __cpp_lib_apply                             201603L
    __cpp_lib_make_from_tuple                   201606L
    __cpp_lib_tuple_element_t                   201402L
    __cpp_lib_tuples_by_type                    201304L

*/

#include <tuple>
#include <cassert>
#include "test_macros.h"

int main()
{
//  ensure that the macros that are supposed to be defined in <tuple> are defined.

/*
#if !defined(__cpp_lib_fooby)
# error "__cpp_lib_fooby is not defined"
#elif __cpp_lib_fooby < 201606L
# error "__cpp_lib_fooby has an invalid value"
#endif
*/
}
