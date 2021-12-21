
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// <memory> feature macros

/*  Constant                                    Value
    __cpp_lib_addressof_constexpr               201603L
    __cpp_lib_allocator_traits_is_always_equal  201411L
    __cpp_lib_enable_shared_from_this           201603L
    __cpp_lib_make_unique                       201304L
    __cpp_lib_raw_memory_algorithms             201606L
    __cpp_lib_shared_ptr_arrays                 201611L
    __cpp_lib_shared_ptr_weak_type              201606L
    __cpp_lib_transparent_operators             201510L

*/

#include <memory>
#include <cassert>
#include "test_macros.h"

int main()
{
//  ensure that the macros that are supposed to be defined in <memory> are defined.

/*
#if !defined(__cpp_lib_fooby)
# error "__cpp_lib_fooby is not defined"
#elif __cpp_lib_fooby < 201606L
# error "__cpp_lib_fooby has an invalid value"
#endif
*/
}
