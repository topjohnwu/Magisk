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
// [simd.casts]
// template <class T, class U, class Abi> see below ex::simd_cast<(const
// ex::simd<U, Abi>&);

#include <experimental/simd>
#include <cstdint>

namespace ex = std::experimental::parallelism_v2;

static_assert(
    std::is_same<decltype(ex::simd_cast<int32_t>(ex::native_simd<int32_t>())),
                 ex::native_simd<int32_t>>::value,
    "");

static_assert(std::is_same<decltype(ex::simd_cast<int64_t>(
                               ex::fixed_size_simd<int32_t, 4>())),
                           ex::fixed_size_simd<int64_t, 4>>::value,
              "");

static_assert(
    std::is_same<decltype(ex::simd_cast<ex::fixed_size_simd<int64_t, 1>>(
                     ex::simd<int32_t, ex::simd_abi::scalar>())),
                 ex::fixed_size_simd<int64_t, 1>>::value,
    "");

static_assert(
    std::is_same<
        decltype(ex::simd_cast<ex::simd<int64_t, ex::simd_abi::scalar>>(
            ex::fixed_size_simd<int32_t, 1>())),
        ex::simd<int64_t, ex::simd_abi::scalar>>::value,
    "");

int main() {}
