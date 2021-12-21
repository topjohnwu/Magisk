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
// template <class T, class U, class Abi> see below ex::static_simd_cast<(const
// ex::simd<U, Abi>&);

#include <experimental/simd>
#include <cstdint>

namespace ex = std::experimental::parallelism_v2;

static_assert(
    std::is_same<decltype(ex::static_simd_cast<float>(ex::native_simd<int>())),
                 ex::native_simd<float>>::value,
    "");

static_assert(
    std::is_same<decltype(ex::static_simd_cast<ex::fixed_size_simd<float, 1>>(
                     ex::simd<int, ex::simd_abi::scalar>())),
                 ex::fixed_size_simd<float, 1>>::value,
    "");

static_assert(
    std::is_same<
        decltype(ex::static_simd_cast<ex::simd<float, ex::simd_abi::scalar>>(
            ex::fixed_size_simd<int, 1>())),
        ex::simd<float, ex::simd_abi::scalar>>::value,
    "");

int main() {}
