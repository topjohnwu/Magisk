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

//  template <class T, class... Types>
//  constexpr add_pointer_t<T> get_if(variant<Types...>* v) noexcept;
// template <class T, class... Types>
//  constexpr add_pointer_t<const T> get_if(const variant<Types...>* v)
//  noexcept;

#include "test_macros.h"
#include "variant_test_helpers.hpp"
#include <cassert>
#include <variant>

void test_const_get_if() {
  {
    using V = std::variant<int>;
    constexpr const V *v = nullptr;
    static_assert(std::get_if<int>(v) == nullptr, "");
  }
  {
    using V = std::variant<int, const long>;
    constexpr V v(42);
    ASSERT_NOEXCEPT(std::get_if<int>(&v));
    ASSERT_SAME_TYPE(decltype(std::get_if<int>(&v)), const int *);
    static_assert(*std::get_if<int>(&v) == 42, "");
    static_assert(std::get_if<const long>(&v) == nullptr, "");
  }
  {
    using V = std::variant<int, const long>;
    constexpr V v(42l);
    ASSERT_SAME_TYPE(decltype(std::get_if<const long>(&v)), const long *);
    static_assert(*std::get_if<const long>(&v) == 42, "");
    static_assert(std::get_if<int>(&v) == nullptr, "");
  }
// FIXME: Remove these once reference support is reinstated
#if !defined(TEST_VARIANT_HAS_NO_REFERENCES)
  {
    using V = std::variant<int &>;
    int x = 42;
    const V v(x);
    ASSERT_SAME_TYPE(decltype(std::get_if<int &>(&v)), int *);
    assert(std::get_if<int &>(&v) == &x);
  }
  {
    using V = std::variant<int &&>;
    int x = 42;
    const V v(std::move(x));
    ASSERT_SAME_TYPE(decltype(std::get_if<int &&>(&v)), int *);
    assert(std::get_if<int &&>(&v) == &x);
  }
  {
    using V = std::variant<const int &&>;
    int x = 42;
    const V v(std::move(x));
    ASSERT_SAME_TYPE(decltype(std::get_if<const int &&>(&v)), const int *);
    assert(std::get_if<const int &&>(&v) == &x);
  }
#endif
}

void test_get_if() {
  {
    using V = std::variant<int>;
    V *v = nullptr;
    assert(std::get_if<int>(v) == nullptr);
  }
  {
    using V = std::variant<int, const long>;
    V v(42);
    ASSERT_NOEXCEPT(std::get_if<int>(&v));
    ASSERT_SAME_TYPE(decltype(std::get_if<int>(&v)), int *);
    assert(*std::get_if<int>(&v) == 42);
    assert(std::get_if<const long>(&v) == nullptr);
  }
  {
    using V = std::variant<int, const long>;
    V v(42l);
    ASSERT_SAME_TYPE(decltype(std::get_if<const long>(&v)), const long *);
    assert(*std::get_if<const long>(&v) == 42);
    assert(std::get_if<int>(&v) == nullptr);
  }
// FIXME: Remove these once reference support is reinstated
#if !defined(TEST_VARIANT_HAS_NO_REFERENCES)
  {
    using V = std::variant<int &>;
    int x = 42;
    V v(x);
    ASSERT_SAME_TYPE(decltype(std::get_if<int &>(&v)), int *);
    assert(std::get_if<int &>(&v) == &x);
  }
  {
    using V = std::variant<const int &>;
    int x = 42;
    V v(x);
    ASSERT_SAME_TYPE(decltype(std::get_if<const int &>(&v)), const int *);
    assert(std::get_if<const int &>(&v) == &x);
  }
  {
    using V = std::variant<int &&>;
    int x = 42;
    V v(std::move(x));
    ASSERT_SAME_TYPE(decltype(std::get_if<int &&>(&v)), int *);
    assert(std::get_if<int &&>(&v) == &x);
  }
  {
    using V = std::variant<const int &&>;
    int x = 42;
    V v(std::move(x));
    ASSERT_SAME_TYPE(decltype(std::get_if<const int &&>(&v)), const int *);
    assert(std::get_if<const int &&>(&v) == &x);
  }
#endif
}

int main() {
  test_const_get_if();
  test_get_if();
}
