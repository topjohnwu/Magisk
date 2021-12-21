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
// [simd.class]
// simd() = default;

#include <cstdint>
#include <experimental/simd>

namespace ex = std::experimental::parallelism_v2;

int main() {
  static_assert(ex::native_simd<int32_t>().size() > 0, "");
  static_assert(ex::fixed_size_simd<int32_t, 4>().size() == 4, "");
  static_assert(ex::fixed_size_simd<int32_t, 5>().size() == 5, "");
  static_assert(ex::fixed_size_simd<int32_t, 1>().size() == 1, "");
  static_assert(ex::fixed_size_simd<char, 32>().size() == 32, "");
}
