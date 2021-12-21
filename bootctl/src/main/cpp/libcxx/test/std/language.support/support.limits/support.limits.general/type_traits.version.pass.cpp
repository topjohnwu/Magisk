
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// <type_traits> feature macros

/*  Constant                                    Value
    __cpp_lib_bool_constant                     201505L
    __cpp_lib_has_unique_object_representations 201606L
    __cpp_lib_integral_constant_callable        201304L
    __cpp_lib_is_aggregate                      201703L
    __cpp_lib_is_final                          201402L
    __cpp_lib_is_invocable                      201703L
    __cpp_lib_is_null_pointer                   201309L
    __cpp_lib_is_swappable                      201603L
    __cpp_lib_logical_traits                    201510L
    __cpp_lib_result_of_sfinae                  201210L
    __cpp_lib_transformation_trait_aliases      201304L
    __cpp_lib_type_trait_variable_templates     201510L
    __cpp_lib_void_t                            201411L

*/

#include <type_traits>
#include <cassert>
#include "test_macros.h"

int main()
{
//  ensure that the macros that are supposed to be defined in <type_traits> are defined.

#if TEST_STD_VER > 14
# if !defined(__cpp_lib_void_t)
#  error "__cpp_lib_void_t is not defined"
# elif __cpp_lib_void_t < 201411L
#  error "__cpp_lib_void_t has an invalid value"
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
