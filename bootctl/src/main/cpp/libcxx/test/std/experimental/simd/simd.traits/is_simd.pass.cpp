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
// template <class T> struct ex::is_simd;
// template <class T> inline constexpr bool ex::is_simd_v =
// ex::is_simd<T>::value;

#include <cstdint>
#include <experimental/simd>
#include "test_macros.h"

namespace ex = std::experimental::parallelism_v2;

struct UserType {};

static_assert(ex::is_simd<ex::native_simd<int8_t>>::value, "");
static_assert(ex::is_simd<ex::native_simd<int16_t>>::value, "");
static_assert(ex::is_simd<ex::native_simd<int32_t>>::value, "");
static_assert(ex::is_simd<ex::native_simd<int64_t>>::value, "");
static_assert(ex::is_simd<ex::native_simd<uint8_t>>::value, "");
static_assert(ex::is_simd<ex::native_simd<uint16_t>>::value, "");
static_assert(ex::is_simd<ex::native_simd<uint32_t>>::value, "");
static_assert(ex::is_simd<ex::native_simd<uint64_t>>::value, "");
static_assert(ex::is_simd<ex::native_simd<float>>::value, "");
static_assert(ex::is_simd<ex::native_simd<double>>::value, "");

static_assert(ex::is_simd<ex::fixed_size_simd<int8_t, 1>>::value, "");
static_assert(ex::is_simd<ex::fixed_size_simd<int16_t, 1>>::value, "");
static_assert(ex::is_simd<ex::fixed_size_simd<int32_t, 1>>::value, "");
static_assert(ex::is_simd<ex::fixed_size_simd<int64_t, 1>>::value, "");
static_assert(ex::is_simd<ex::fixed_size_simd<uint8_t, 1>>::value, "");
static_assert(ex::is_simd<ex::fixed_size_simd<uint16_t, 1>>::value, "");
static_assert(ex::is_simd<ex::fixed_size_simd<uint32_t, 1>>::value, "");
static_assert(ex::is_simd<ex::fixed_size_simd<uint64_t, 1>>::value, "");
static_assert(ex::is_simd<ex::fixed_size_simd<float, 1>>::value, "");
static_assert(ex::is_simd<ex::fixed_size_simd<double, 1>>::value, "");

static_assert(ex::is_simd<ex::fixed_size_simd<int8_t, 3>>::value, "");
static_assert(ex::is_simd<ex::fixed_size_simd<int16_t, 3>>::value, "");
static_assert(ex::is_simd<ex::fixed_size_simd<int32_t, 3>>::value, "");
static_assert(ex::is_simd<ex::fixed_size_simd<int64_t, 3>>::value, "");
static_assert(ex::is_simd<ex::fixed_size_simd<uint8_t, 3>>::value, "");
static_assert(ex::is_simd<ex::fixed_size_simd<uint16_t, 3>>::value, "");
static_assert(ex::is_simd<ex::fixed_size_simd<uint32_t, 3>>::value, "");
static_assert(ex::is_simd<ex::fixed_size_simd<uint64_t, 3>>::value, "");
static_assert(ex::is_simd<ex::fixed_size_simd<float, 3>>::value, "");
static_assert(ex::is_simd<ex::fixed_size_simd<double, 3>>::value, "");

static_assert(ex::is_simd<ex::fixed_size_simd<int8_t, 32>>::value, "");
static_assert(ex::is_simd<ex::fixed_size_simd<int16_t, 32>>::value, "");
static_assert(ex::is_simd<ex::fixed_size_simd<int32_t, 32>>::value, "");
static_assert(ex::is_simd<ex::fixed_size_simd<int64_t, 32>>::value, "");
static_assert(ex::is_simd<ex::fixed_size_simd<uint8_t, 32>>::value, "");
static_assert(ex::is_simd<ex::fixed_size_simd<uint16_t, 32>>::value, "");
static_assert(ex::is_simd<ex::fixed_size_simd<uint32_t, 32>>::value, "");
static_assert(ex::is_simd<ex::fixed_size_simd<uint64_t, 32>>::value, "");
static_assert(ex::is_simd<ex::fixed_size_simd<float, 32>>::value, "");
static_assert(ex::is_simd<ex::fixed_size_simd<double, 32>>::value, "");

static_assert(!ex::is_simd<void>::value, "");
static_assert(!ex::is_simd<int>::value, "");
static_assert(!ex::is_simd<float>::value, "");
static_assert(!ex::is_simd<ex::simd_mask<int>>::value, "");
static_assert(!ex::is_simd<ex::simd_mask<float>>::value, "");
static_assert(!ex::is_simd<UserType>::value, "");

static_assert(ex::is_simd_v<ex::native_simd<int8_t>>, "");
static_assert(ex::is_simd_v<ex::native_simd<int16_t>>, "");
static_assert(ex::is_simd_v<ex::native_simd<int32_t>>, "");
static_assert(ex::is_simd_v<ex::native_simd<int64_t>>, "");
static_assert(ex::is_simd_v<ex::native_simd<uint8_t>>, "");
static_assert(ex::is_simd_v<ex::native_simd<uint16_t>>, "");
static_assert(ex::is_simd_v<ex::native_simd<uint32_t>>, "");
static_assert(ex::is_simd_v<ex::native_simd<uint64_t>>, "");
static_assert(ex::is_simd_v<ex::native_simd<float>>, "");
static_assert(ex::is_simd_v<ex::native_simd<double>>, "");

static_assert(ex::is_simd_v<ex::fixed_size_simd<int8_t, 1>>, "");
static_assert(ex::is_simd_v<ex::fixed_size_simd<int16_t, 1>>, "");
static_assert(ex::is_simd_v<ex::fixed_size_simd<int32_t, 1>>, "");
static_assert(ex::is_simd_v<ex::fixed_size_simd<int64_t, 1>>, "");
static_assert(ex::is_simd_v<ex::fixed_size_simd<uint8_t, 1>>, "");
static_assert(ex::is_simd_v<ex::fixed_size_simd<uint16_t, 1>>, "");
static_assert(ex::is_simd_v<ex::fixed_size_simd<uint32_t, 1>>, "");
static_assert(ex::is_simd_v<ex::fixed_size_simd<uint64_t, 1>>, "");
static_assert(ex::is_simd_v<ex::fixed_size_simd<float, 1>>, "");
static_assert(ex::is_simd_v<ex::fixed_size_simd<double, 1>>, "");

static_assert(ex::is_simd_v<ex::fixed_size_simd<int8_t, 3>>, "");
static_assert(ex::is_simd_v<ex::fixed_size_simd<int16_t, 3>>, "");
static_assert(ex::is_simd_v<ex::fixed_size_simd<int32_t, 3>>, "");
static_assert(ex::is_simd_v<ex::fixed_size_simd<int64_t, 3>>, "");
static_assert(ex::is_simd_v<ex::fixed_size_simd<uint8_t, 3>>, "");
static_assert(ex::is_simd_v<ex::fixed_size_simd<uint16_t, 3>>, "");
static_assert(ex::is_simd_v<ex::fixed_size_simd<uint32_t, 3>>, "");
static_assert(ex::is_simd_v<ex::fixed_size_simd<uint64_t, 3>>, "");
static_assert(ex::is_simd_v<ex::fixed_size_simd<float, 3>>, "");
static_assert(ex::is_simd_v<ex::fixed_size_simd<double, 3>>, "");

static_assert(ex::is_simd_v<ex::fixed_size_simd<int8_t, 32>>, "");
static_assert(ex::is_simd_v<ex::fixed_size_simd<int16_t, 32>>, "");
static_assert(ex::is_simd_v<ex::fixed_size_simd<int32_t, 32>>, "");
static_assert(ex::is_simd_v<ex::fixed_size_simd<int64_t, 32>>, "");
static_assert(ex::is_simd_v<ex::fixed_size_simd<uint8_t, 32>>, "");
static_assert(ex::is_simd_v<ex::fixed_size_simd<uint16_t, 32>>, "");
static_assert(ex::is_simd_v<ex::fixed_size_simd<uint32_t, 32>>, "");
static_assert(ex::is_simd_v<ex::fixed_size_simd<uint64_t, 32>>, "");
static_assert(ex::is_simd_v<ex::fixed_size_simd<float, 32>>, "");
static_assert(ex::is_simd_v<ex::fixed_size_simd<double, 32>>, "");

static_assert(!ex::is_simd_v<void>, "");
static_assert(!ex::is_simd_v<int>, "");
static_assert(!ex::is_simd_v<float>, "");
static_assert(!ex::is_simd_v<ex::simd_mask<int>>, "");
static_assert(!ex::is_simd_v<ex::simd_mask<float>>, "");
static_assert(!ex::is_simd_v<UserType>, "");

int main() {}
