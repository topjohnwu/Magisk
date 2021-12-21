
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// <algorithm> feature macros

/*  Constant                                    Value
    __cpp_lib_clamp                             201603L
    __cpp_lib_constexpr_swap_algorithms         201806L
    __cpp_lib_parallel_algorithm                201603L
    __cpp_lib_robust_nonmodifying_seq_ops       201304L
    __cpp_lib_sample                            201603L

*/

#include <algorithm>
#include <cassert>
#include "test_macros.h"

int main()
{
//  ensure that the macros that are supposed to be defined in <algorithm> are defined.

/*
#if !defined(__cpp_lib_fooby)
# error "__cpp_lib_fooby is not defined"
#elif __cpp_lib_fooby < 201606L
# error "__cpp_lib_fooby has an invalid value"
#endif
*/
}
