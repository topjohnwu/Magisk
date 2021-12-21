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
// template <class U> simd(U&& value);

#include <cstdint>
#include <experimental/simd>

namespace ex = std::experimental::parallelism_v2;

template <class T, class... Args>
auto not_supported_native_simd_ctor(Args&&... args)
    -> decltype(ex::native_simd<T>(std::forward<Args>(args)...),
                void()) = delete;

template <class T>
void not_supported_native_simd_ctor(...) {}

template <class T, class... Args>
auto supported_native_simd_ctor(Args&&... args)
    -> decltype(ex::native_simd<T>(std::forward<Args>(args)...), void()) {}

template <class T>
void supported_native_simd_ctor(...) = delete;

void compile_narrowing_conversion() {
  supported_native_simd_ctor<int8_t>(3);
  supported_native_simd_ctor<int16_t>(3);
  supported_native_simd_ctor<int32_t>(3);
  supported_native_simd_ctor<int64_t>(3);
  supported_native_simd_ctor<uint8_t>(3);
  supported_native_simd_ctor<uint16_t>(3);
  supported_native_simd_ctor<uint32_t>(3);
  supported_native_simd_ctor<uint64_t>(3);
  supported_native_simd_ctor<float>(3.f);
  supported_native_simd_ctor<double>(3.);
  supported_native_simd_ctor<long double>(3.);

  not_supported_native_simd_ctor<float>(3.);
  not_supported_native_simd_ctor<int8_t>(long(3));
  not_supported_native_simd_ctor<float>(long(3));
  not_supported_native_simd_ctor<int>(3.);
}

void compile_convertible() {
  struct ConvertibleToInt {
    operator int64_t() const;
  };
  supported_native_simd_ctor<int64_t>(ConvertibleToInt());

  struct NotConvertibleToInt {};
  not_supported_native_simd_ctor<int64_t>(NotConvertibleToInt());
}

void compile_unsigned() {
  not_supported_native_simd_ctor<int>(3u);
  supported_native_simd_ctor<uint16_t>(3u);
}

template <typename SimdType>
void test_broadcast() {
  SimdType a(3);
  for (size_t i = 0; i < a.size(); i++) {
    assert(a[i] == 3);
  }
}

int main() {
  test_broadcast<ex::native_simd<int>>();
  test_broadcast<ex::fixed_size_simd<int, 4>>();
  test_broadcast<ex::fixed_size_simd<int, 15>>();
}
