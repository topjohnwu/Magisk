//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14

// <experimental/simd>
//
// [simd.traits]
// template <class T> struct is_simd_flag_type;
// template <class T> inline constexpr bool ex::is_simd_flag_type_v =
// ex::is_simd_flag_type<T>::value;

#include <cstdint>
#include <experimental/simd>
#include "test_macros.h"

namespace ex = std::experimental::parallelism_v2;

struct UserType {};

static_assert(ex::is_simd_flag_type<ex::element_aligned_tag>::value, "");
static_assert(ex::is_simd_flag_type<ex::vector_aligned_tag>::value, "");
static_assert(ex::is_simd_flag_type<ex::overaligned_tag<16>>::value, "");
static_assert(ex::is_simd_flag_type<ex::overaligned_tag<32>>::value, "");

static_assert(!ex::is_simd_flag_type<void>::value, "");
static_assert(!ex::is_simd_flag_type<int>::value, "");
static_assert(!ex::is_simd_flag_type<float>::value, "");
static_assert(!ex::is_simd_flag_type<UserType>::value, "");
static_assert(!ex::is_simd_flag_type<ex::simd<int8_t>>::value, "");
static_assert(!ex::is_simd_flag_type<ex::simd_mask<int8_t>>::value, "");

static_assert(ex::is_simd_flag_type_v<ex::element_aligned_tag>, "");
static_assert(ex::is_simd_flag_type_v<ex::vector_aligned_tag>, "");
static_assert(ex::is_simd_flag_type_v<ex::overaligned_tag<16>>, "");
static_assert(ex::is_simd_flag_type_v<ex::overaligned_tag<32>>, "");

static_assert(!ex::is_simd_flag_type_v<void>, "");
static_assert(!ex::is_simd_flag_type_v<int>, "");
static_assert(!ex::is_simd_flag_type_v<float>, "");
static_assert(!ex::is_simd_flag_type_v<UserType>, "");
static_assert(!ex::is_simd_flag_type_v<ex::simd<int8_t>>, "");
static_assert(!ex::is_simd_flag_type_v<ex::simd_mask<int8_t>>, "");

int main() {}
