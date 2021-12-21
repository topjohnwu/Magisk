// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14

// <variant>

// template <class T> struct variant_size; // undefined
// template <class T> struct variant_size<const T>;
// template <class T> struct variant_size<volatile T>;
// template <class T> struct variant_size<const volatile T>;
// template <class T> constexpr size_t variant_size_v
//     = variant_size<T>::value;

#include <memory>
#include <type_traits>
#include <variant>

template <class V, size_t E> void test() {
  static_assert(std::variant_size<V>::value == E, "");
  static_assert(std::variant_size<const V>::value == E, "");
  static_assert(std::variant_size<volatile V>::value == E, "");
  static_assert(std::variant_size<const volatile V>::value == E, "");
  static_assert(std::variant_size_v<V> == E, "");
  static_assert(std::variant_size_v<const V> == E, "");
  static_assert(std::variant_size_v<volatile V> == E, "");
  static_assert(std::variant_size_v<const volatile V> == E, "");
  static_assert(std::is_base_of<std::integral_constant<std::size_t, E>,
                                std::variant_size<V>>::value,
                "");
};

int main() {
  test<std::variant<>, 0>();
  test<std::variant<void *>, 1>();
  test<std::variant<long, long, void *, double>, 4>();
}
