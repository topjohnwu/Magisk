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

// template <class T, class... Types> constexpr T& get(variant<Types...>& v);
// template <class T, class... Types> constexpr T&& get(variant<Types...>&& v);
// template <class T, class... Types> constexpr const T& get(const
// variant<Types...>& v);
// template <class T, class... Types> constexpr const T&& get(const
// variant<Types...>&& v);

#include "test_macros.h"
#include "variant_test_helpers.hpp"
#include <cassert>
#include <type_traits>
#include <utility>
#include <variant>

void test_const_lvalue_get() {
  {
    using V = std::variant<int, const long>;
    constexpr V v(42);
#ifndef __clang__ // Avoid https://bugs.llvm.org/show_bug.cgi?id=15481
    ASSERT_NOEXCEPT(std::get<int>(v));
#endif
    ASSERT_SAME_TYPE(decltype(std::get<0>(v)), const int &);
    static_assert(std::get<int>(v) == 42, "");
  }
  {
    using V = std::variant<int, const long>;
    const V v(42);
    ASSERT_NOT_NOEXCEPT(std::get<int>(v));
    ASSERT_SAME_TYPE(decltype(std::get<0>(v)), const int &);
    assert(std::get<int>(v) == 42);
  }
  {
    using V = std::variant<int, const long>;
    constexpr V v(42l);
#ifndef __clang__ // Avoid https://bugs.llvm.org/show_bug.cgi?id=15481
    ASSERT_NOEXCEPT(std::get<const long>(v));
#endif
    ASSERT_SAME_TYPE(decltype(std::get<const long>(v)), const long &);
    static_assert(std::get<const long>(v) == 42, "");
  }
  {
    using V = std::variant<int, const long>;
    const V v(42l);
    ASSERT_NOT_NOEXCEPT(std::get<const long>(v));
    ASSERT_SAME_TYPE(decltype(std::get<const long>(v)), const long &);
    assert(std::get<const long>(v) == 42);
  }
// FIXME: Remove these once reference support is reinstated
#if !defined(TEST_VARIANT_HAS_NO_REFERENCES)
  {
    using V = std::variant<int &>;
    int x = 42;
    const V v(x);
    ASSERT_SAME_TYPE(decltype(std::get<int &>(v)), int &);
    assert(&std::get<int &>(v) == &x);
  }
  {
    using V = std::variant<int &&>;
    int x = 42;
    const V v(std::move(x));
    ASSERT_SAME_TYPE(decltype(std::get<int &&>(v)), int &);
    assert(&std::get<int &&>(v) == &x);
  }
  {
    using V = std::variant<const int &&>;
    int x = 42;
    const V v(std::move(x));
    ASSERT_SAME_TYPE(decltype(std::get<const int &&>(v)), const int &);
    assert(&std::get<const int &&>(v) == &x);
  }
#endif
}

void test_lvalue_get() {
  {
    using V = std::variant<int, const long>;
    V v(42);
    ASSERT_NOT_NOEXCEPT(std::get<int>(v));
    ASSERT_SAME_TYPE(decltype(std::get<int>(v)), int &);
    assert(std::get<int>(v) == 42);
  }
  {
    using V = std::variant<int, const long>;
    V v(42l);
    ASSERT_SAME_TYPE(decltype(std::get<const long>(v)), const long &);
    assert(std::get<const long>(v) == 42);
  }
// FIXME: Remove these once reference support is reinstated
#if !defined(TEST_VARIANT_HAS_NO_REFERENCES)
  {
    using V = std::variant<int &>;
    int x = 42;
    V v(x);
    ASSERT_SAME_TYPE(decltype(std::get<int &>(v)), int &);
    assert(&std::get<int &>(v) == &x);
  }
  {
    using V = std::variant<const int &>;
    int x = 42;
    V v(x);
    ASSERT_SAME_TYPE(decltype(std::get<const int &>(v)), const int &);
    assert(&std::get<const int &>(v) == &x);
  }
  {
    using V = std::variant<int &&>;
    int x = 42;
    V v(std::move(x));
    ASSERT_SAME_TYPE(decltype(std::get<int &&>(v)), int &);
    assert(&std::get<int &&>(v) == &x);
  }
  {
    using V = std::variant<const int &&>;
    int x = 42;
    V v(std::move(x));
    ASSERT_SAME_TYPE(decltype(std::get<const int &&>(v)), const int &);
    assert(&std::get<const int &&>(v) == &x);
  }
#endif
}

void test_rvalue_get() {
  {
    using V = std::variant<int, const long>;
    V v(42);
    ASSERT_NOT_NOEXCEPT(std::get<int>(std::move(v)));
    ASSERT_SAME_TYPE(decltype(std::get<int>(std::move(v))), int &&);
    assert(std::get<int>(std::move(v)) == 42);
  }
  {
    using V = std::variant<int, const long>;
    V v(42l);
    ASSERT_SAME_TYPE(decltype(std::get<const long>(std::move(v))),
                     const long &&);
    assert(std::get<const long>(std::move(v)) == 42);
  }
// FIXME: Remove these once reference support is reinstated
#if !defined(TEST_VARIANT_HAS_NO_REFERENCES)
  {
    using V = std::variant<int &>;
    int x = 42;
    V v(x);
    ASSERT_SAME_TYPE(decltype(std::get<int &>(std::move(v))), int &);
    assert(&std::get<int &>(std::move(v)) == &x);
  }
  {
    using V = std::variant<const int &>;
    int x = 42;
    V v(x);
    ASSERT_SAME_TYPE(decltype(std::get<const int &>(std::move(v))),
                     const int &);
    assert(&std::get<const int &>(std::move(v)) == &x);
  }
  {
    using V = std::variant<int &&>;
    int x = 42;
    V v(std::move(x));
    ASSERT_SAME_TYPE(decltype(std::get<int &&>(std::move(v))), int &&);
    int &&xref = std::get<int &&>(std::move(v));
    assert(&xref == &x);
  }
  {
    using V = std::variant<const int &&>;
    int x = 42;
    V v(std::move(x));
    ASSERT_SAME_TYPE(decltype(std::get<const int &&>(std::move(v))),
                     const int &&);
    const int &&xref = std::get<const int &&>(std::move(v));
    assert(&xref == &x);
  }
#endif
}

void test_const_rvalue_get() {
  {
    using V = std::variant<int, const long>;
    const V v(42);
    ASSERT_NOT_NOEXCEPT(std::get<int>(std::move(v)));
    ASSERT_SAME_TYPE(decltype(std::get<int>(std::move(v))), const int &&);
    assert(std::get<int>(std::move(v)) == 42);
  }
  {
    using V = std::variant<int, const long>;
    const V v(42l);
    ASSERT_SAME_TYPE(decltype(std::get<const long>(std::move(v))),
                     const long &&);
    assert(std::get<const long>(std::move(v)) == 42);
  }
// FIXME: Remove these once reference support is reinstated
#if !defined(TEST_VARIANT_HAS_NO_REFERENCES)
  {
    using V = std::variant<int &>;
    int x = 42;
    const V v(x);
    ASSERT_SAME_TYPE(decltype(std::get<int &>(std::move(v))), int &);
    assert(&std::get<int &>(std::move(v)) == &x);
  }
  {
    using V = std::variant<const int &>;
    int x = 42;
    const V v(x);
    ASSERT_SAME_TYPE(decltype(std::get<const int &>(std::move(v))),
                     const int &);
    assert(&std::get<const int &>(std::move(v)) == &x);
  }
  {
    using V = std::variant<int &&>;
    int x = 42;
    const V v(std::move(x));
    ASSERT_SAME_TYPE(decltype(std::get<int &&>(std::move(v))), int &&);
    int &&xref = std::get<int &&>(std::move(v));
    assert(&xref == &x);
  }
  {
    using V = std::variant<const int &&>;
    int x = 42;
    const V v(std::move(x));
    ASSERT_SAME_TYPE(decltype(std::get<const int &&>(std::move(v))),
                     const int &&);
    const int &&xref = std::get<const int &&>(std::move(v));
    assert(&xref == &x);
  }
#endif
}

template <class Tp> struct identity { using type = Tp; };

void test_throws_for_all_value_categories() {
#ifndef TEST_HAS_NO_EXCEPTIONS
  using V = std::variant<int, long>;
  V v0(42);
  const V &cv0 = v0;
  assert(v0.index() == 0);
  V v1(42l);
  const V &cv1 = v1;
  assert(v1.index() == 1);
  identity<int> zero;
  identity<long> one;
  auto test = [](auto idx, auto &&v) {
    using Idx = decltype(idx);
    try {
      TEST_IGNORE_NODISCARD std::get<typename Idx::type>(std::forward<decltype(v)>(v));
    } catch (const std::bad_variant_access &) {
      return true;
    } catch (...) { /* ... */
    }
    return false;
  };
  { // lvalue test cases
    assert(test(one, v0));
    assert(test(zero, v1));
  }
  { // const lvalue test cases
    assert(test(one, cv0));
    assert(test(zero, cv1));
  }
  { // rvalue test cases
    assert(test(one, std::move(v0)));
    assert(test(zero, std::move(v1)));
  }
  { // const rvalue test cases
    assert(test(one, std::move(cv0)));
    assert(test(zero, std::move(cv1)));
  }
#endif
}

int main() {
  test_const_lvalue_get();
  test_lvalue_get();
  test_rvalue_get();
  test_const_rvalue_get();
  test_throws_for_all_value_categories();
}
