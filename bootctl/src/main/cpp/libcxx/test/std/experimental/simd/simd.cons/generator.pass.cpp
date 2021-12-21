//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14

// See GCC PR63723.
// UNSUPPORTED: gcc-4.9

// <experimental/simd>
//
// [simd.class]
// template <class G> explicit simd(G&& gen);

#include <experimental/simd>
#include <cstdint>

namespace ex = std::experimental::parallelism_v2;

template <class T, class... Args>
auto not_supported_simd128_ctor(Args&&... args) -> decltype(
    ex::fixed_size_simd<T, 16 / sizeof(T)>(std::forward<Args>(args)...),
    void()) = delete;

template <class T>
void not_supported_simd128_ctor(...) {}

template <class T, class... Args>
auto supported_simd128_ctor(Args&&... args) -> decltype(
    ex::fixed_size_simd<T, 16 / sizeof(T)>(std::forward<Args>(args)...),
    void()) {}

template <class T>
void supported_simd128_ctor(...) = delete;

struct identity {
  template <size_t value>
  int operator()(std::integral_constant<size_t, value>) const {
    return value;
  }
};

void compile_generator() {
  supported_simd128_ctor<int>(identity());
  not_supported_simd128_ctor<int>([](int i) { return float(i); });
  not_supported_simd128_ctor<int>([](intptr_t i) { return (int*)(i); });
  not_supported_simd128_ctor<int>([](int* i) { return i; });
}

struct limited_identity {
  template <size_t value>
  typename std::conditional<value <= 2, int32_t, int64_t>::type
  operator()(std::integral_constant<size_t, value>) const {
    return value;
  }
};

void compile_limited_identity() {
  supported_simd128_ctor<int64_t>(limited_identity());
  not_supported_simd128_ctor<int32_t>(limited_identity());
}

template <typename SimdType>
void test_generator() {
  {
    SimdType a([](int i) { return i; });
    assert(a[0] == 0);
    assert(a[1] == 1);
    assert(a[2] == 2);
    assert(a[3] == 3);
  }
  {
    SimdType a([](int i) { return 2 * i - 1; });
    assert(a[0] == -1);
    assert(a[1] == 1);
    assert(a[2] == 3);
    assert(a[3] == 5);
  }
}

int main() {
  // TODO: adjust the tests when this assertion fails.
  assert(ex::native_simd<int32_t>::size() >= 4);
  test_generator<ex::native_simd<int32_t>>();
  test_generator<ex::fixed_size_simd<int32_t, 4>>();
}
