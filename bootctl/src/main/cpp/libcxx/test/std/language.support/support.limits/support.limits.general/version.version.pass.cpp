
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// <version> feature macros

/*  Constant                                    Value
    __cpp_lib_addressof_constexpr               201603L
    __cpp_lib_allocator_traits_is_always_equal  201411L
    __cpp_lib_any                               201606L
    __cpp_lib_apply                             201603L
    __cpp_lib_array_constexpr                   201603L
    __cpp_lib_as_const                          201510L
    __cpp_lib_atomic_is_always_lock_free        201603L
    __cpp_lib_atomic_ref                        201806L
    __cpp_lib_bit_cast                          201806L
    __cpp_lib_bool_constant                     201505L
    __cpp_lib_boyer_moore_searcher              201603L
    __cpp_lib_byte                              201603L
    __cpp_lib_chrono                            201611L
    __cpp_lib_chrono_udls                       201304L
    __cpp_lib_clamp                             201603L
    __cpp_lib_complex_udls                      201309L
    __cpp_lib_concepts                          201806L
    __cpp_lib_constexpr_swap_algorithms         201806L
    __cpp_lib_destroying_delete                 201806L
    __cpp_lib_enable_shared_from_this           201603L
    __cpp_lib_exchange_function                 201304L
    __cpp_lib_execution                         201603L
    __cpp_lib_filesystem                        201703L
    __cpp_lib_gcd_lcm                           201606L
    __cpp_lib_generic_associative_lookup        201304L
    __cpp_lib_hardware_interference_size        201703L
    __cpp_lib_has_unique_object_representations 201606L
    __cpp_lib_hypot                             201603L
    __cpp_lib_incomplete_container_elements     201505L
    __cpp_lib_integer_sequence                  201304L
    __cpp_lib_integral_constant_callable        201304L
    __cpp_lib_invoke                            201411L
    __cpp_lib_is_aggregate                      201703L
    __cpp_lib_is_final                          201402L
    __cpp_lib_is_invocable                      201703L
    __cpp_lib_is_null_pointer                   201309L
    __cpp_lib_is_swappable                      201603L
    __cpp_lib_launder                           201606L
    __cpp_lib_list_remove_return_type           201806L
    __cpp_lib_logical_traits                    201510L
    __cpp_lib_make_from_tuple                   201606L
    __cpp_lib_make_reverse_iterator             201402L
    __cpp_lib_make_unique                       201304L
    __cpp_lib_map_try_emplace                   201411L
    __cpp_lib_math_special_functions            201603L
    __cpp_lib_memory_resource                   201603L
    __cpp_lib_node_extract                      201606L
    __cpp_lib_nonmember_container_access        201411L
    __cpp_lib_not_fn                            201603L
    __cpp_lib_null_iterators                    201304L
    __cpp_lib_optional                          201606L
    __cpp_lib_parallel_algorithm                201603L
    __cpp_lib_quoted_string_io                  201304L
    __cpp_lib_raw_memory_algorithms             201606L
    __cpp_lib_result_of_sfinae                  201210L
    __cpp_lib_robust_nonmodifying_seq_ops       201304L
    __cpp_lib_sample                            201603L
    __cpp_lib_scoped_lock                       201703L
    __cpp_lib_shared_mutex                      201505L
    __cpp_lib_shared_ptr_arrays                 201611L
    __cpp_lib_shared_ptr_weak_type              201606L
    __cpp_lib_shared_timed_mutex                201402L
    __cpp_lib_string_udls                       201304L
    __cpp_lib_string_view                       201606L
    __cpp_lib_to_chars                          201611L
    __cpp_lib_three_way_comparison              201711L
    __cpp_lib_transformation_trait_aliases      201304L
    __cpp_lib_transparent_operators             201510L
    __cpp_lib_tuple_element_t                   201402L
    __cpp_lib_tuples_by_type                    201304L
    __cpp_lib_type_trait_variable_templates     201510L
    __cpp_lib_uncaught_exceptions               201411L
    __cpp_lib_unordered_map_try_emplace         201411L
    __cpp_lib_variant                           201606L
    __cpp_lib_void_t                            201411L

*/

#include <version>
#include <cassert>
#include "test_macros.h"

int main()
{
//  ensure that the macros that are supposed to be defined in <version> are defined.

#if TEST_STD_VER > 14
# if !defined(__cpp_lib_atomic_is_always_lock_free)
#  error "__cpp_lib_atomic_is_always_lock_free is not defined"
# elif __cpp_lib_atomic_is_always_lock_free < 201603L
#  error "__cpp_lib_atomic_is_always_lock_free has an invalid value"
# endif
#endif

#if TEST_STD_VER > 14
# if !defined(__cpp_lib_filesystem)
#  error "__cpp_lib_filesystem is not defined"
# elif __cpp_lib_filesystem < 201703L
#  error "__cpp_lib_filesystem has an invalid value"
# endif
#endif

#if TEST_STD_VER > 14
# if !defined(__cpp_lib_invoke)
#  error "__cpp_lib_invoke is not defined"
# elif __cpp_lib_invoke < 201411L
#  error "__cpp_lib_invoke has an invalid value"
# endif
#endif

#if TEST_STD_VER > 14
# if !defined(__cpp_lib_void_t)
#  error "__cpp_lib_void_t is not defined"
# elif __cpp_lib_void_t < 201411L
#  error "__cpp_lib_void_t has an invalid value"
# endif
#endif

#if TEST_STD_VER > 17 && defined(__cpp_char8_t)
# if !defined(__cpp_lib_char8_t)  
  LIBCPP_STATIC_ASSERT(false, "__cpp_lib_char8_t is not defined");
# else
#  if __cpp_lib_char8_t < 201811L
#   error "__cpp_lib_char8_t has an invalid value"
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
