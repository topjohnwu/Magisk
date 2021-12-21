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
// loads [simd.load]
// template <class U, class Flags> void copy_from(const U* mem, Flags f);

#include <experimental/simd>
#include <cstdint>

#include "test_macros.h"

namespace ex = std::experimental::parallelism_v2;

template <class T, class... Args>
auto not_supported_load(Args&&... args) -> decltype(
    std::declval<ex::native_simd<T>>().copy_from(std::forward<Args>(args)...),
    void()) = delete;

template <class T>
void not_supported_load(...) {}

template <class T, class... Args>
auto supported_load(Args&&... args) -> decltype(
    std::declval<ex::native_simd<T>>().copy_from(std::forward<Args>(args)...),
    void()) {}

template <class T>
void supported_load(...) = delete;

void compile_load() {
  supported_load<int>((int*)nullptr, ex::element_aligned_tag());
  supported_load<uint32_t>((int*)nullptr, ex::element_aligned_tag());
  supported_load<double>((float*)nullptr, ex::element_aligned_tag());
  supported_load<uint16_t>((unsigned int*)nullptr, ex::element_aligned_tag());
  supported_load<uint32_t>((float*)nullptr, ex::element_aligned_tag());

  not_supported_load<int>((int*)nullptr, int());
}

template <typename SimdType>
void test_load() {
  alignas(32) int32_t buffer[] = {4, 3, 2, 1};
  {
    SimdType a;
    a.copy_from(buffer, ex::element_aligned_tag());
    assert(a[0] == 4);
    assert(a[1] == 3);
    assert(a[2] == 2);
    assert(a[3] == 1);
  }
  {
    SimdType a;
    a.copy_from(buffer, ex::vector_aligned_tag());
    assert(a[0] == 4);
    assert(a[1] == 3);
    assert(a[2] == 2);
    assert(a[3] == 1);
  }
  {
    SimdType a;
    a.copy_from(buffer, ex::overaligned_tag<32>());
    assert(a[0] == 4);
    assert(a[1] == 3);
    assert(a[2] == 2);
    assert(a[3] == 1);
  }

  {
    SimdType a;
    a.copy_from(buffer, ex::element_aligned);
    assert(a[0] == 4);
    assert(a[1] == 3);
    assert(a[2] == 2);
    assert(a[3] == 1);
  }
  {
    SimdType a;
    a.copy_from(buffer, ex::vector_aligned);
    assert(a[0] == 4);
    assert(a[1] == 3);
    assert(a[2] == 2);
    assert(a[3] == 1);
  }
  {
    SimdType a;
    a.copy_from(buffer, ex::overaligned<32>);
    assert(a[0] == 4);
    assert(a[1] == 3);
    assert(a[2] == 2);
    assert(a[3] == 1);
  }
}

template <typename SimdType>
void test_converting_load() {
  float buffer[] = {1., 2., 4., 8.};
  SimdType a;
  a.copy_from(buffer, ex::element_aligned_tag());
  assert(a[0] == 1);
  assert(a[1] == 2);
  assert(a[2] == 4);
  assert(a[3] == 8);
}

int main() {
  // TODO: adjust the tests when this assertion fails.
  assert(ex::native_simd<int32_t>::size() >= 4);
  test_load<ex::native_simd<int32_t>>();
  test_load<ex::fixed_size_simd<int32_t, 4>>();
  test_converting_load<ex::native_simd<int32_t>>();
  test_converting_load<ex::fixed_size_simd<int32_t, 4>>();
}
