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
// [simd.abi]

#include <experimental/simd>
#include <cstdint>

namespace ex = std::experimental::parallelism_v2;

constexpr inline int reg_width() {
#if defined(__AVX__)
  return 32;
#else
  return 16;
#endif
}

#ifndef _LIBCPP_HAS_NO_VECTOR_EXTENSION

static_assert(
    sizeof(ex::simd<char, ex::__simd_abi<ex::_StorageKind::_VecExt, 1>>) == 1,
    "");
static_assert(
    sizeof(ex::simd<char, ex::__simd_abi<ex::_StorageKind::_VecExt, 2>>) == 2,
    "");
static_assert(
    sizeof(ex::simd<char, ex::__simd_abi<ex::_StorageKind::_VecExt, 3>>) == 4,
    "");
static_assert(
    sizeof(ex::simd<char, ex::__simd_abi<ex::_StorageKind::_VecExt, 12>>) == 16,
    "");
static_assert(
    sizeof(ex::simd<int32_t, ex::__simd_abi<ex::_StorageKind::_VecExt, 3>>) ==
        16,
    "");
static_assert(
    sizeof(ex::simd<int32_t, ex::__simd_abi<ex::_StorageKind::_VecExt, 5>>) ==
        32,
    "");
static_assert(
    std::is_same<ex::simd_abi::native<int8_t>,
                 ex::__simd_abi<ex::_StorageKind::_VecExt, reg_width()>>::value,
    "");
#else
static_assert(
    std::is_same<ex::simd_abi::native<int8_t>,
                 ex::__simd_abi<ex::_StorageKind::_Array, reg_width()>>::value,
    "");

#endif

static_assert(std::is_same<ex::simd_abi::compatible<int8_t>,
                           ex::__simd_abi<ex::_StorageKind::_Array, 16>>::value,
              "");

int main() {}
