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
// template <class T> struct ex::is_simd_mask;
// template <class T> inline constexpr bool ex::is_simd_mask_v =
// ex::is_simd_mask<T>::value;

#include <cstdint>
#include <experimental/simd>
#include "test_macros.h"

namespace ex = std::experimental::parallelism_v2;

struct UserType {};

static_assert(ex::is_simd_mask<ex::native_simd_mask<int8_t>>::value, "");
static_assert(ex::is_simd_mask<ex::native_simd_mask<int16_t>>::value, "");
static_assert(ex::is_simd_mask<ex::native_simd_mask<int32_t>>::value, "");
static_assert(ex::is_simd_mask<ex::native_simd_mask<int64_t>>::value, "");
static_assert(ex::is_simd_mask<ex::native_simd_mask<uint8_t>>::value, "");
static_assert(ex::is_simd_mask<ex::native_simd_mask<uint16_t>>::value, "");
static_assert(ex::is_simd_mask<ex::native_simd_mask<uint32_t>>::value, "");
static_assert(ex::is_simd_mask<ex::native_simd_mask<uint64_t>>::value, "");
static_assert(ex::is_simd_mask<ex::native_simd_mask<float>>::value, "");
static_assert(ex::is_simd_mask<ex::native_simd_mask<double>>::value, "");

static_assert(ex::is_simd_mask<ex::fixed_size_simd_mask<int8_t, 1>>::value, "");
static_assert(ex::is_simd_mask<ex::fixed_size_simd_mask<int16_t, 1>>::value,
              "");
static_assert(ex::is_simd_mask<ex::fixed_size_simd_mask<int32_t, 1>>::value,
              "");
static_assert(ex::is_simd_mask<ex::fixed_size_simd_mask<int64_t, 1>>::value,
              "");
static_assert(ex::is_simd_mask<ex::fixed_size_simd_mask<uint8_t, 1>>::value,
              "");
static_assert(ex::is_simd_mask<ex::fixed_size_simd_mask<uint16_t, 1>>::value,
              "");
static_assert(ex::is_simd_mask<ex::fixed_size_simd_mask<uint32_t, 1>>::value,
              "");
static_assert(ex::is_simd_mask<ex::fixed_size_simd_mask<uint64_t, 1>>::value,
              "");
static_assert(ex::is_simd_mask<ex::fixed_size_simd_mask<float, 1>>::value, "");
static_assert(ex::is_simd_mask<ex::fixed_size_simd_mask<double, 1>>::value, "");

static_assert(ex::is_simd_mask<ex::fixed_size_simd_mask<int8_t, 3>>::value, "");
static_assert(ex::is_simd_mask<ex::fixed_size_simd_mask<int16_t, 3>>::value,
              "");
static_assert(ex::is_simd_mask<ex::fixed_size_simd_mask<int32_t, 3>>::value,
              "");
static_assert(ex::is_simd_mask<ex::fixed_size_simd_mask<int64_t, 3>>::value,
              "");
static_assert(ex::is_simd_mask<ex::fixed_size_simd_mask<uint8_t, 3>>::value,
              "");
static_assert(ex::is_simd_mask<ex::fixed_size_simd_mask<uint16_t, 3>>::value,
              "");
static_assert(ex::is_simd_mask<ex::fixed_size_simd_mask<uint32_t, 3>>::value,
              "");
static_assert(ex::is_simd_mask<ex::fixed_size_simd_mask<uint64_t, 3>>::value,
              "");
static_assert(ex::is_simd_mask<ex::fixed_size_simd_mask<float, 3>>::value, "");
static_assert(ex::is_simd_mask<ex::fixed_size_simd_mask<double, 3>>::value, "");

static_assert(ex::is_simd_mask<ex::fixed_size_simd_mask<int8_t, 32>>::value,
              "");
static_assert(ex::is_simd_mask<ex::fixed_size_simd_mask<int16_t, 32>>::value,
              "");
static_assert(ex::is_simd_mask<ex::fixed_size_simd_mask<int32_t, 32>>::value,
              "");
static_assert(ex::is_simd_mask<ex::fixed_size_simd_mask<int64_t, 32>>::value,
              "");
static_assert(ex::is_simd_mask<ex::fixed_size_simd_mask<uint8_t, 32>>::value,
              "");
static_assert(ex::is_simd_mask<ex::fixed_size_simd_mask<uint16_t, 32>>::value,
              "");
static_assert(ex::is_simd_mask<ex::fixed_size_simd_mask<uint32_t, 32>>::value,
              "");
static_assert(ex::is_simd_mask<ex::fixed_size_simd_mask<uint64_t, 32>>::value,
              "");
static_assert(ex::is_simd_mask<ex::fixed_size_simd_mask<float, 32>>::value, "");
static_assert(ex::is_simd_mask<ex::fixed_size_simd_mask<double, 32>>::value,
              "");

static_assert(!ex::is_simd_mask<void>::value, "");
static_assert(!ex::is_simd_mask<int>::value, "");
static_assert(!ex::is_simd_mask<float>::value, "");
static_assert(!ex::is_simd_mask<ex::simd<int>>::value, "");
static_assert(!ex::is_simd_mask<ex::simd<float>>::value, "");
static_assert(!ex::is_simd_mask<UserType>::value, "");

static_assert(ex::is_simd_mask_v<ex::native_simd_mask<int8_t>>, "");
static_assert(ex::is_simd_mask_v<ex::native_simd_mask<int16_t>>, "");
static_assert(ex::is_simd_mask_v<ex::native_simd_mask<int32_t>>, "");
static_assert(ex::is_simd_mask_v<ex::native_simd_mask<int64_t>>, "");
static_assert(ex::is_simd_mask_v<ex::native_simd_mask<uint8_t>>, "");
static_assert(ex::is_simd_mask_v<ex::native_simd_mask<uint16_t>>, "");
static_assert(ex::is_simd_mask_v<ex::native_simd_mask<uint32_t>>, "");
static_assert(ex::is_simd_mask_v<ex::native_simd_mask<uint64_t>>, "");
static_assert(ex::is_simd_mask_v<ex::native_simd_mask<float>>, "");
static_assert(ex::is_simd_mask_v<ex::native_simd_mask<double>>, "");

static_assert(ex::is_simd_mask_v<ex::fixed_size_simd_mask<int8_t, 1>>, "");
static_assert(ex::is_simd_mask_v<ex::fixed_size_simd_mask<int16_t, 1>>, "");
static_assert(ex::is_simd_mask_v<ex::fixed_size_simd_mask<int32_t, 1>>, "");
static_assert(ex::is_simd_mask_v<ex::fixed_size_simd_mask<int64_t, 1>>, "");
static_assert(ex::is_simd_mask_v<ex::fixed_size_simd_mask<uint8_t, 1>>, "");
static_assert(ex::is_simd_mask_v<ex::fixed_size_simd_mask<uint16_t, 1>>, "");
static_assert(ex::is_simd_mask_v<ex::fixed_size_simd_mask<uint32_t, 1>>, "");
static_assert(ex::is_simd_mask_v<ex::fixed_size_simd_mask<uint64_t, 1>>, "");
static_assert(ex::is_simd_mask_v<ex::fixed_size_simd_mask<float, 1>>, "");
static_assert(ex::is_simd_mask_v<ex::fixed_size_simd_mask<double, 1>>, "");

static_assert(ex::is_simd_mask_v<ex::fixed_size_simd_mask<int8_t, 3>>, "");
static_assert(ex::is_simd_mask_v<ex::fixed_size_simd_mask<int16_t, 3>>, "");
static_assert(ex::is_simd_mask_v<ex::fixed_size_simd_mask<int32_t, 3>>, "");
static_assert(ex::is_simd_mask_v<ex::fixed_size_simd_mask<int64_t, 3>>, "");
static_assert(ex::is_simd_mask_v<ex::fixed_size_simd_mask<uint8_t, 3>>, "");
static_assert(ex::is_simd_mask_v<ex::fixed_size_simd_mask<uint16_t, 3>>, "");
static_assert(ex::is_simd_mask_v<ex::fixed_size_simd_mask<uint32_t, 3>>, "");
static_assert(ex::is_simd_mask_v<ex::fixed_size_simd_mask<uint64_t, 3>>, "");
static_assert(ex::is_simd_mask_v<ex::fixed_size_simd_mask<float, 3>>, "");
static_assert(ex::is_simd_mask_v<ex::fixed_size_simd_mask<double, 3>>, "");

static_assert(ex::is_simd_mask_v<ex::fixed_size_simd_mask<int8_t, 32>>, "");
static_assert(ex::is_simd_mask_v<ex::fixed_size_simd_mask<int16_t, 32>>, "");
static_assert(ex::is_simd_mask_v<ex::fixed_size_simd_mask<int32_t, 32>>, "");
static_assert(ex::is_simd_mask_v<ex::fixed_size_simd_mask<int64_t, 32>>, "");
static_assert(ex::is_simd_mask_v<ex::fixed_size_simd_mask<uint8_t, 32>>, "");
static_assert(ex::is_simd_mask_v<ex::fixed_size_simd_mask<uint16_t, 32>>, "");
static_assert(ex::is_simd_mask_v<ex::fixed_size_simd_mask<uint32_t, 32>>, "");
static_assert(ex::is_simd_mask_v<ex::fixed_size_simd_mask<uint64_t, 32>>, "");
static_assert(ex::is_simd_mask_v<ex::fixed_size_simd_mask<float, 32>>, "");
static_assert(ex::is_simd_mask_v<ex::fixed_size_simd_mask<double, 32>>, "");

static_assert(!ex::is_simd_mask_v<void>, "");
static_assert(!ex::is_simd_mask_v<int>, "");
static_assert(!ex::is_simd_mask_v<float>, "");
static_assert(!ex::is_simd_mask_v<ex::simd<int>>, "");
static_assert(!ex::is_simd_mask_v<ex::simd<float>>, "");
static_assert(!ex::is_simd_mask_v<UserType>, "");

int main() {}
