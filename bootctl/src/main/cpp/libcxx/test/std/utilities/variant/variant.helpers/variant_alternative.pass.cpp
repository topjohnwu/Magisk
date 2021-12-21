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

// template <size_t I, class T> struct variant_alternative; // undefined
// template <size_t I, class T> struct variant_alternative<I, const T>;
// template <size_t I, class T> struct variant_alternative<I, volatile T>;
// template <size_t I, class T> struct variant_alternative<I, const volatile T>;
// template <size_t I, class T>
//   using variant_alternative_t = typename variant_alternative<I, T>::type;
//
// template <size_t I, class... Types>
//    struct variant_alternative<I, variant<Types...>>;

#include <memory>
#include <type_traits>
#include <variant>

#include "test_macros.h"
#include "variant_test_helpers.hpp"

template <class V, size_t I, class E> void test() {
  static_assert(
      std::is_same_v<typename std::variant_alternative<I, V>::type, E>, "");
  static_assert(
      std::is_same_v<typename std::variant_alternative<I, const V>::type,
                     const E>,
      "");
  static_assert(
      std::is_same_v<typename std::variant_alternative<I, volatile V>::type,
                     volatile E>,
      "");
  static_assert(
      std::is_same_v<
          typename std::variant_alternative<I, const volatile V>::type,
          const volatile E>,
      "");
  static_assert(std::is_same_v<std::variant_alternative_t<I, V>, E>, "");
  static_assert(std::is_same_v<std::variant_alternative_t<I, const V>, const E>,
                "");
  static_assert(
      std::is_same_v<std::variant_alternative_t<I, volatile V>, volatile E>,
      "");
  static_assert(std::is_same_v<std::variant_alternative_t<I, const volatile V>,
                               const volatile E>,
                "");
}

int main() {
  {
    using V = std::variant<int, void *, const void *, long double>;
    test<V, 0, int>();
    test<V, 1, void *>();
    test<V, 2, const void *>();
    test<V, 3, long double>();
  }
#if !defined(TEST_VARIANT_HAS_NO_REFERENCES)
  {
    using V = std::variant<int, int &, const int &, int &&, long double>;
    test<V, 0, int>();
    test<V, 1, int &>();
    test<V, 2, const int &>();
    test<V, 3, int &&>();
    test<V, 4, long double>();
  }
#endif
}
