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
// // stores [simd.store]
// template <class U, class Flags> void copy_to(U* mem, Flags f) const;

#include <experimental/simd>
#include <cstdint>

#include "test_macros.h"

namespace ex = std::experimental::parallelism_v2;

template <typename SimdType>
void test_store() {
  SimdType a([](int i) { return 4 - i; });
  {
    alignas(32) int32_t buffer[4] = {0};
    a.copy_to(buffer, ex::element_aligned_tag());
    assert(buffer[0] == 4);
    assert(buffer[1] == 3);
    assert(buffer[2] == 2);
    assert(buffer[3] == 1);
  }
  {
    alignas(32) int32_t buffer[4] = {0};
    a.copy_to(buffer, ex::vector_aligned_tag());
    assert(buffer[0] == 4);
    assert(buffer[1] == 3);
    assert(buffer[2] == 2);
    assert(buffer[3] == 1);
  }
  {
    alignas(32) int32_t buffer[4] = {0};
    a.copy_to(buffer, ex::overaligned_tag<32>());
    assert(buffer[0] == 4);
    assert(buffer[1] == 3);
    assert(buffer[2] == 2);
    assert(buffer[3] == 1);
  }

  {
    alignas(32) int32_t buffer[4] = {0};
    a.copy_to(buffer, ex::element_aligned);
    assert(buffer[0] == 4);
    assert(buffer[1] == 3);
    assert(buffer[2] == 2);
    assert(buffer[3] == 1);
  }
  {
    alignas(32) int32_t buffer[4] = {0};
    a.copy_to(buffer, ex::vector_aligned);
    assert(buffer[0] == 4);
    assert(buffer[1] == 3);
    assert(buffer[2] == 2);
    assert(buffer[3] == 1);
  }
  {
    alignas(32) int32_t buffer[4] = {0};
    a.copy_to(buffer, ex::overaligned<32>);
    assert(buffer[0] == 4);
    assert(buffer[1] == 3);
    assert(buffer[2] == 2);
    assert(buffer[3] == 1);
  }
}

template <typename SimdType>
void test_converting_store() {
  float buffer[4] = {0.};
  SimdType a([](int i) { return 1 << i; });
  a.copy_to(buffer, ex::element_aligned_tag());
  assert(buffer[0] == 1.);
  assert(buffer[1] == 2.);
  assert(buffer[2] == 4.);
  assert(buffer[3] == 8.);
}

int main() {
  // TODO: adjust the tests when this assertion fails.
  test_store<ex::native_simd<int32_t>>();
  test_store<ex::fixed_size_simd<int32_t, 4>>();
  test_converting_store<ex::native_simd<int32_t>>();
  test_converting_store<ex::fixed_size_simd<int32_t, 4>>();
}
