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

// XFAIL: availability=macosx10.13
// XFAIL: availability=macosx10.12
// XFAIL: availability=macosx10.11
// XFAIL: availability=macosx10.10
// XFAIL: availability=macosx10.9
// XFAIL: availability=macosx10.8
// XFAIL: availability=macosx10.7

// <variant>

// template <class ...Types> class variant;

// template <class T, class ...Args> T& emplace(Args&&... args);

#include <cassert>
#include <string>
#include <type_traits>
#include <variant>

#include "archetypes.hpp"
#include "test_convertible.hpp"
#include "test_macros.h"
#include "variant_test_helpers.hpp"

template <class Var, class T, class... Args>
constexpr auto test_emplace_exists_imp(int) -> decltype(
    std::declval<Var>().template emplace<T>(std::declval<Args>()...), true) {
  return true;
}

template <class, class, class...>
constexpr auto test_emplace_exists_imp(long) -> bool {
  return false;
}

template <class... Args> constexpr bool emplace_exists() {
  return test_emplace_exists_imp<Args...>(0);
}

void test_emplace_sfinae() {
  {
    using V = std::variant<int, void *, const void *, TestTypes::NoCtors>;
    static_assert(emplace_exists<V, int>(), "");
    static_assert(emplace_exists<V, int, int>(), "");
    static_assert(!emplace_exists<V, int, decltype(nullptr)>(),
                  "cannot construct");
    static_assert(emplace_exists<V, void *, decltype(nullptr)>(), "");
    static_assert(!emplace_exists<V, void *, int>(), "cannot construct");
    static_assert(emplace_exists<V, void *, int *>(), "");
    static_assert(!emplace_exists<V, void *, const int *>(), "");
    static_assert(emplace_exists<V, const void *, const int *>(), "");
    static_assert(emplace_exists<V, const void *, int *>(), "");
    static_assert(!emplace_exists<V, TestTypes::NoCtors>(), "cannot construct");
  }
#if !defined(TEST_VARIANT_HAS_NO_REFERENCES)
  using V = std::variant<int, int &, const int &, int &&, long, long,
                         TestTypes::NoCtors>;
  static_assert(emplace_exists<V, int>(), "");
  static_assert(emplace_exists<V, int, int>(), "");
  static_assert(emplace_exists<V, int, long long>(), "");
  static_assert(!emplace_exists<V, int, int, int>(), "too many args");
  static_assert(emplace_exists<V, int &, int &>(), "");
  static_assert(!emplace_exists<V, int &>(), "cannot default construct ref");
  static_assert(!emplace_exists<V, int &, const int &>(), "cannot bind ref");
  static_assert(!emplace_exists<V, int &, int &&>(), "cannot bind ref");
  static_assert(emplace_exists<V, const int &, int &>(), "");
  static_assert(emplace_exists<V, const int &, const int &>(), "");
  static_assert(emplace_exists<V, const int &, int &&>(), "");
  static_assert(!emplace_exists<V, const int &, void *>(),
                "not constructible from void*");
  static_assert(emplace_exists<V, int &&, int>(), "");
  static_assert(!emplace_exists<V, int &&, int &>(), "cannot bind ref");
  static_assert(!emplace_exists<V, int &&, const int &>(), "cannot bind ref");
  static_assert(!emplace_exists<V, int &&, const int &&>(), "cannot bind ref");
  static_assert(!emplace_exists<V, long, long>(), "ambiguous");
  static_assert(!emplace_exists<V, TestTypes::NoCtors>(),
                "cannot construct void");
#endif
}

void test_basic() {
  {
    using V = std::variant<int>;
    V v(42);
    auto& ref1 = v.emplace<int>();
    static_assert(std::is_same_v<int&, decltype(ref1)>, "");
    assert(std::get<0>(v) == 0);
    assert(&ref1 == &std::get<0>(v));
    auto& ref2 = v.emplace<int>(42);
    static_assert(std::is_same_v<int&, decltype(ref2)>, "");
    assert(std::get<0>(v) == 42);
    assert(&ref2 == &std::get<0>(v));
  }
  {
    using V =
        std::variant<int, long, const void *, TestTypes::NoCtors, std::string>;
    const int x = 100;
    V v(std::in_place_type<int>, -1);
    // default emplace a value
    auto& ref1 = v.emplace<long>();
    static_assert(std::is_same_v<long&, decltype(ref1)>, "");
    assert(std::get<1>(v) == 0);
    assert(&ref1 == &std::get<1>(v));
    auto& ref2 = v.emplace<const void *>(&x);
    static_assert(std::is_same_v<const void *&, decltype(ref2)>, "");
    assert(std::get<2>(v) == &x);
    assert(&ref2 == &std::get<2>(v));
    // emplace with multiple args
    auto& ref3 = v.emplace<std::string>(3, 'a');
    static_assert(std::is_same_v<std::string&, decltype(ref3)>, "");
    assert(std::get<4>(v) == "aaa");
    assert(&ref3 == &std::get<4>(v));
  }
#if !defined(TEST_VARIANT_HAS_NO_REFERENCES)
  {
    using V = std::variant<int, long, const int &, int &&, TestTypes::NoCtors,
                           std::string>;
    const int x = 100;
    int y = 42;
    int z = 43;
    V v(std::in_place_index<0>, -1);
    // default emplace a value
    auto& ref1 = v.emplace<long>();
    static_assert(std::is_same_v<long&, decltype(ref1)>, "");
    assert(std::get<long>(v) == 0);
    assert(&ref1 == &std::get<long>(v));
    // emplace a reference
    auto& ref2 = v.emplace<const int &>(x);
    static_assert(std::is_same_v<const int&, decltype(ref2)>, "");
    assert(&std::get<const int &>(v) == &x);
    assert(&ref2 == &std::get<const int &>(v));
    // emplace an rvalue reference
    auto& ref3 = v.emplace<int &&>(std::move(y));
    static_assert(std::is_same_v<int &&, decltype(ref3)>, "");
    assert(&std::get<int &&>(v) == &y);
    assert(&ref3 == &std::get<int &&>(v));
    // re-emplace a new reference over the active member
    auto& ref4 = v.emplace<int &&>(std::move(z));
    static_assert(std::is_same_v<int &, decltype(ref4)>, "");
    assert(&std::get<int &&>(v) == &z);
    assert(&ref4 == &std::get<int &&>(v));
    // emplace with multiple args
    auto& ref5 = v.emplace<std::string>(3, 'a');
    static_assert(std::is_same_v<std::string&, decltype(ref5)>, "");
    assert(std::get<std::string>(v) == "aaa");
    assert(&ref5 == &std::get<std::string>(v));
  }
#endif
}

int main() {
  test_basic();
  test_emplace_sfinae();
}
