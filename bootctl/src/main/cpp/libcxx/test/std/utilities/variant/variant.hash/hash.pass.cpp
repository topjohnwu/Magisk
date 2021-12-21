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

// template <class... Types> struct hash<variant<Types...>>;
// template <> struct hash<monostate>;

#include <cassert>
#include <type_traits>
#include <variant>

#include "test_macros.h"
#include "variant_test_helpers.hpp"
#include "poisoned_hash_helper.hpp"

#ifndef TEST_HAS_NO_EXCEPTIONS
namespace std {
template <> struct hash<::MakeEmptyT> {
  size_t operator()(const ::MakeEmptyT &) const {
    assert(false);
    return 0;
  }
};
}
#endif

void test_hash_variant() {
  {
    using V = std::variant<int, long, int>;
    using H = std::hash<V>;
    const V v(std::in_place_index<0>, 42);
    const V v_copy = v;
    V v2(std::in_place_index<0>, 100);
    const H h{};
    assert(h(v) == h(v));
    assert(h(v) != h(v2));
    assert(h(v) == h(v_copy));
    {
      ASSERT_SAME_TYPE(decltype(h(v)), std::size_t);
      static_assert(std::is_copy_constructible<H>::value, "");
    }
  }
  {
    using V = std::variant<std::monostate, int, long, const char *>;
    using H = std::hash<V>;
    const char *str = "hello";
    const V v0;
    const V v0_other;
    const V v1(42);
    const V v1_other(100);
    V v2(100l);
    V v2_other(999l);
    V v3(str);
    V v3_other("not hello");
    const H h{};
    assert(h(v0) == h(v0));
    assert(h(v0) == h(v0_other));
    assert(h(v1) == h(v1));
    assert(h(v1) != h(v1_other));
    assert(h(v2) == h(v2));
    assert(h(v2) != h(v2_other));
    assert(h(v3) == h(v3));
    assert(h(v3) != h(v3_other));
    assert(h(v0) != h(v1));
    assert(h(v0) != h(v2));
    assert(h(v0) != h(v3));
    assert(h(v1) != h(v2));
    assert(h(v1) != h(v3));
    assert(h(v2) != h(v3));
  }
#ifndef TEST_HAS_NO_EXCEPTIONS
  {
    using V = std::variant<int, MakeEmptyT>;
    using H = std::hash<V>;
    V v;
    makeEmpty(v);
    V v2;
    makeEmpty(v2);
    const H h{};
    assert(h(v) == h(v2));
  }
#endif
}

void test_hash_monostate() {
  using H = std::hash<std::monostate>;
  const H h{};
  std::monostate m1{};
  const std::monostate m2{};
  assert(h(m1) == h(m1));
  assert(h(m2) == h(m2));
  assert(h(m1) == h(m2));
  {
    ASSERT_SAME_TYPE(decltype(h(m1)), std::size_t);
    ASSERT_NOEXCEPT(h(m1));
    static_assert(std::is_copy_constructible<H>::value, "");
  }
  {
    test_hash_enabled_for_type<std::monostate>();
  }
}

void test_hash_variant_duplicate_elements() {
    // Test that the index of the alternative participates in the hash value.
    using V = std::variant<std::monostate, std::monostate>;
    using H = std::hash<V>;
    H h{};
    const V v1(std::in_place_index<0>);
    const V v2(std::in_place_index<1>);
    assert(h(v1) == h(v1));
    assert(h(v2) == h(v2));
    LIBCPP_ASSERT(h(v1) != h(v2));
}

struct A {};
struct B {};

namespace std {

template <>
struct hash<B> {
  size_t operator()(B const&) const {
    return 0;
  }
};

}

void test_hash_variant_enabled() {
  {
    test_hash_enabled_for_type<std::variant<int> >();
    test_hash_enabled_for_type<std::variant<int*, long, double, const int> >();
  }
  {
    test_hash_disabled_for_type<std::variant<int, A>>();
    test_hash_disabled_for_type<std::variant<const A, void*>>();
  }
  {
    test_hash_enabled_for_type<std::variant<int, B>>();
    test_hash_enabled_for_type<std::variant<const B, int>>();
  }
}

int main() {
  test_hash_variant();
  test_hash_variant_duplicate_elements();
  test_hash_monostate();
  test_hash_variant_enabled();
}
