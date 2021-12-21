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

// XFAIL: availability=macosx10.13
// XFAIL: availability=macosx10.12
// XFAIL: availability=macosx10.11
// XFAIL: availability=macosx10.10
// XFAIL: availability=macosx10.9
// XFAIL: availability=macosx10.8
// XFAIL: availability=macosx10.7

// template <class ...Types> class variant;

// template <size_t I, class Up, class ...Args>
// constexpr explicit
// variant(in_place_index_t<I>, initializer_list<Up>, Args&&...);

#include <cassert>
#include <string>
#include <type_traits>
#include <variant>

#include "test_convertible.hpp"
#include "test_macros.h"

struct InitList {
  std::size_t size;
  constexpr InitList(std::initializer_list<int> il) : size(il.size()) {}
};

struct InitListArg {
  std::size_t size;
  int value;
  constexpr InitListArg(std::initializer_list<int> il, int v)
      : size(il.size()), value(v) {}
};

void test_ctor_sfinae() {
  using IL = std::initializer_list<int>;
  { // just init list
    using V = std::variant<InitList, InitListArg, int>;
    static_assert(std::is_constructible<V, std::in_place_index_t<0>, IL>::value,
                  "");
    static_assert(!test_convertible<V, std::in_place_index_t<0>, IL>(), "");
  }
  { // too many arguments
    using V = std::variant<InitList, InitListArg, int>;
    static_assert(
        !std::is_constructible<V, std::in_place_index_t<0>, IL, int>::value,
        "");
    static_assert(!test_convertible<V, std::in_place_index_t<0>, IL, int>(),
                  "");
  }
  { // too few arguments
    using V = std::variant<InitList, InitListArg, int>;
    static_assert(
        !std::is_constructible<V, std::in_place_index_t<1>, IL>::value, "");
    static_assert(!test_convertible<V, std::in_place_index_t<1>, IL>(), "");
  }
  { // init list and arguments
    using V = std::variant<InitList, InitListArg, int>;
    static_assert(
        std::is_constructible<V, std::in_place_index_t<1>, IL, int>::value, "");
    static_assert(!test_convertible<V, std::in_place_index_t<1>, IL, int>(),
                  "");
  }
  { // not constructible from arguments
    using V = std::variant<InitList, InitListArg, int>;
    static_assert(
        !std::is_constructible<V, std::in_place_index_t<2>, IL>::value, "");
    static_assert(!test_convertible<V, std::in_place_index_t<2>, IL>(), "");
  }
  { // index not in variant
    using V = std::variant<InitList, InitListArg, int>;
    static_assert(
        !std::is_constructible<V, std::in_place_index_t<3>, IL>::value, "");
    static_assert(!test_convertible<V, std::in_place_index_t<3>, IL>(), "");
  }
}

void test_ctor_basic() {
  {
    constexpr std::variant<InitList, InitListArg, InitList> v(
        std::in_place_index<0>, {1, 2, 3});
    static_assert(v.index() == 0, "");
    static_assert(std::get<0>(v).size == 3, "");
  }
  {
    constexpr std::variant<InitList, InitListArg, InitList> v(
        std::in_place_index<2>, {1, 2, 3});
    static_assert(v.index() == 2, "");
    static_assert(std::get<2>(v).size == 3, "");
  }
  {
    constexpr std::variant<InitList, InitListArg, InitListArg> v(
        std::in_place_index<1>, {1, 2, 3, 4}, 42);
    static_assert(v.index() == 1, "");
    static_assert(std::get<1>(v).size == 4, "");
    static_assert(std::get<1>(v).value == 42, "");
  }
}

int main() {
  test_ctor_basic();
  test_ctor_sfinae();
}
