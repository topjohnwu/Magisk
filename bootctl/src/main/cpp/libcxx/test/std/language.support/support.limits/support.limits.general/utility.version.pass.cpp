
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// <utility> feature macros

/*  Constant                                    Value
    __cpp_lib_as_const                          201510L
    __cpp_lib_exchange_function                 201304L
    __cpp_lib_integer_sequence                  201304L
    __cpp_lib_tuples_by_type                    201304L

*/

#include <utility>
#include <cassert>
#include "test_macros.h"

int main()
{
//  ensure that the macros that are supposed to be defined in <utility> are defined.

/*
#if !defined(__cpp_lib_fooby)
# error "__cpp_lib_fooby is not defined"
#elif __cpp_lib_fooby < 201606L
# error "__cpp_lib_fooby has an invalid value"
#endif
*/
}
