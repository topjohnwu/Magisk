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

// template <class ...Types>
// constexpr bool
// operator==(variant<Types...> const&, variant<Types...> const&) noexcept;
//
// template <class ...Types>
// constexpr bool
// operator!=(variant<Types...> const&, variant<Types...> const&) noexcept;
//
// template <class ...Types>
// constexpr bool
// operator<(variant<Types...> const&, variant<Types...> const&) noexcept;
//
// template <class ...Types>
// constexpr bool
// operator>(variant<Types...> const&, variant<Types...> const&) noexcept;
//
// template <class ...Types>
// constexpr bool
// operator<=(variant<Types...> const&, variant<Types...> const&) noexcept;
//
// template <class ...Types>
// constexpr bool
// operator>=(variant<Types...> const&, variant<Types...> const&) noexcept;

#include <cassert>
#include <type_traits>
#include <utility>
#include <variant>

#include "test_macros.h"

#ifndef TEST_HAS_NO_EXCEPTIONS
struct MakeEmptyT {
  MakeEmptyT() = default;
  MakeEmptyT(MakeEmptyT &&) { throw 42; }
  MakeEmptyT &operator=(MakeEmptyT &&) { throw 42; }
};
inline bool operator==(const MakeEmptyT &, const MakeEmptyT &) {
  assert(false);
  return false;
}
inline bool operator!=(const MakeEmptyT &, const MakeEmptyT &) {
  assert(false);
  return false;
}
inline bool operator<(const MakeEmptyT &, const MakeEmptyT &) {
  assert(false);
  return false;
}
inline bool operator<=(const MakeEmptyT &, const MakeEmptyT &) {
  assert(false);
  return false;
}
inline bool operator>(const MakeEmptyT &, const MakeEmptyT &) {
  assert(false);
  return false;
}
inline bool operator>=(const MakeEmptyT &, const MakeEmptyT &) {
  assert(false);
  return false;
}

template <class Variant> void makeEmpty(Variant &v) {
  Variant v2(std::in_place_type<MakeEmptyT>);
  try {
    v = std::move(v2);
    assert(false);
  } catch (...) {
    assert(v.valueless_by_exception());
  }
}
#endif // TEST_HAS_NO_EXCEPTIONS

struct MyBool {
  bool value;
  constexpr explicit MyBool(bool v) : value(v) {}
  constexpr operator bool() const noexcept { return value; }
};

struct ComparesToMyBool {
  int value = 0;
};
inline constexpr MyBool operator==(const ComparesToMyBool& LHS, const ComparesToMyBool& RHS) noexcept {
  return MyBool(LHS.value == RHS.value);
}
inline constexpr MyBool operator!=(const ComparesToMyBool& LHS, const ComparesToMyBool& RHS) noexcept {
  return MyBool(LHS.value != RHS.value);
}
inline constexpr MyBool operator<(const ComparesToMyBool& LHS, const ComparesToMyBool& RHS) noexcept {
  return MyBool(LHS.value < RHS.value);
}
inline constexpr MyBool operator<=(const ComparesToMyBool& LHS, const ComparesToMyBool& RHS) noexcept {
  return MyBool(LHS.value <= RHS.value);
}
inline constexpr MyBool operator>(const ComparesToMyBool& LHS, const ComparesToMyBool& RHS) noexcept {
  return MyBool(LHS.value > RHS.value);
}
inline constexpr MyBool operator>=(const ComparesToMyBool& LHS, const ComparesToMyBool& RHS) noexcept {
  return MyBool(LHS.value >= RHS.value);
}

template <class T1, class T2>
void test_equality_basic() {
  {
    using V = std::variant<T1, T2>;
    constexpr V v1(std::in_place_index<0>, T1{42});
    constexpr V v2(std::in_place_index<0>, T1{42});
    static_assert(v1 == v2, "");
    static_assert(v2 == v1, "");
    static_assert(!(v1 != v2), "");
    static_assert(!(v2 != v1), "");
  }
  {
    using V = std::variant<T1, T2>;
    constexpr V v1(std::in_place_index<0>, T1{42});
    constexpr V v2(std::in_place_index<0>, T1{43});
    static_assert(!(v1 == v2), "");
    static_assert(!(v2 == v1), "");
    static_assert(v1 != v2, "");
    static_assert(v2 != v1, "");
  }
  {
    using V = std::variant<T1, T2>;
    constexpr V v1(std::in_place_index<0>, T1{42});
    constexpr V v2(std::in_place_index<1>, T2{42});
    static_assert(!(v1 == v2), "");
    static_assert(!(v2 == v1), "");
    static_assert(v1 != v2, "");
    static_assert(v2 != v1, "");
  }
  {
    using V = std::variant<T1, T2>;
    constexpr V v1(std::in_place_index<1>, T2{42});
    constexpr V v2(std::in_place_index<1>, T2{42});
    static_assert(v1 == v2, "");
    static_assert(v2 == v1, "");
    static_assert(!(v1 != v2), "");
    static_assert(!(v2 != v1), "");
  }
}

void test_equality() {
  test_equality_basic<int, long>();
  test_equality_basic<ComparesToMyBool, int>();
  test_equality_basic<int, ComparesToMyBool>();
  test_equality_basic<ComparesToMyBool, ComparesToMyBool>();
#ifndef TEST_HAS_NO_EXCEPTIONS
  {
    using V = std::variant<int, MakeEmptyT>;
    V v1;
    V v2;
    makeEmpty(v2);
    assert(!(v1 == v2));
    assert(!(v2 == v1));
    assert(v1 != v2);
    assert(v2 != v1);
  }
  {
    using V = std::variant<int, MakeEmptyT>;
    V v1;
    makeEmpty(v1);
    V v2;
    assert(!(v1 == v2));
    assert(!(v2 == v1));
    assert(v1 != v2);
    assert(v2 != v1);
  }
  {
    using V = std::variant<int, MakeEmptyT>;
    V v1;
    makeEmpty(v1);
    V v2;
    makeEmpty(v2);
    assert(v1 == v2);
    assert(v2 == v1);
    assert(!(v1 != v2));
    assert(!(v2 != v1));
  }
#endif
}

template <class Var>
constexpr bool test_less(const Var &l, const Var &r, bool expect_less,
                         bool expect_greater) {
  static_assert(std::is_same_v<decltype(l < r), bool>, "");
  static_assert(std::is_same_v<decltype(l <= r), bool>, "");
  static_assert(std::is_same_v<decltype(l > r), bool>, "");
  static_assert(std::is_same_v<decltype(l >= r), bool>, "");

  return ((l < r) == expect_less) && (!(l >= r) == expect_less) &&
         ((l > r) == expect_greater) && (!(l <= r) == expect_greater);
}

template <class T1, class T2>
void test_relational_basic() {
  { // same index, same value
    using V = std::variant<T1, T2>;
    constexpr V v1(std::in_place_index<0>, T1{1});
    constexpr V v2(std::in_place_index<0>, T1{1});
    static_assert(test_less(v1, v2, false, false), "");
  }
  { // same index, value < other_value
    using V = std::variant<T1, T2>;
    constexpr V v1(std::in_place_index<0>, T1{0});
    constexpr V v2(std::in_place_index<0>, T1{1});
    static_assert(test_less(v1, v2, true, false), "");
  }
  { // same index, value > other_value
    using V = std::variant<T1, T2>;
    constexpr V v1(std::in_place_index<0>, T1{1});
    constexpr V v2(std::in_place_index<0>, T1{0});
    static_assert(test_less(v1, v2, false, true), "");
  }
  { // LHS.index() < RHS.index()
    using V = std::variant<T1, T2>;
    constexpr V v1(std::in_place_index<0>, T1{0});
    constexpr V v2(std::in_place_index<1>, T2{0});
    static_assert(test_less(v1, v2, true, false), "");
  }
  { // LHS.index() > RHS.index()
    using V = std::variant<T1, T2>;
    constexpr V v1(std::in_place_index<1>, T2{0});
    constexpr V v2(std::in_place_index<0>, T1{0});
    static_assert(test_less(v1, v2, false, true), "");
  }
}

void test_relational() {
  test_relational_basic<int, long>();
  test_relational_basic<ComparesToMyBool, int>();
  test_relational_basic<int, ComparesToMyBool>();
  test_relational_basic<ComparesToMyBool, ComparesToMyBool>();
#ifndef TEST_HAS_NO_EXCEPTIONS
  { // LHS.index() < RHS.index(), RHS is empty
    using V = std::variant<int, MakeEmptyT>;
    V v1;
    V v2;
    makeEmpty(v2);
    assert(test_less(v1, v2, false, true));
  }
  { // LHS.index() > RHS.index(), LHS is empty
    using V = std::variant<int, MakeEmptyT>;
    V v1;
    makeEmpty(v1);
    V v2;
    assert(test_less(v1, v2, true, false));
  }
  { // LHS.index() == RHS.index(), LHS and RHS are empty
    using V = std::variant<int, MakeEmptyT>;
    V v1;
    makeEmpty(v1);
    V v2;
    makeEmpty(v2);
    assert(test_less(v1, v2, false, false));
  }
#endif
}

int main() {
  test_equality();
  test_relational();
}
