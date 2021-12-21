
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// <functional> feature macros

/*  Constant                                    Value
    __cpp_lib_boyer_moore_searcher              201603L
    __cpp_lib_invoke                            201411L
    __cpp_lib_not_fn                            201603L
    __cpp_lib_result_of_sfinae                  201210L
    __cpp_lib_transparent_operators             201510L

*/

#include <functional>
#include <cassert>
#include "test_macros.h"

int main()
{
//  ensure that the macros that are supposed to be defined in <functional> are defined.

#if TEST_STD_VER > 14
# if !defined(__cpp_lib_invoke)
#  error "__cpp_lib_invoke is not defined"
# elif __cpp_lib_invoke < 201411L
#  error "__cpp_lib_invoke has an invalid value"
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
