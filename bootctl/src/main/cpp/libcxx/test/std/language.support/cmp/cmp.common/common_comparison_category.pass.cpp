//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14, c++17

// <compare>

// template <class ...Ts> struct common_comparison_category
// template <class ...Ts> using common_comparison_category_t


#include <compare>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

const volatile void* volatile sink;

template <class Expect, class ...Args>
void test_cat() {
  using Cat = std::common_comparison_category<Args...>;
  using CatT = typename Cat::type;
  static_assert(std::is_same<CatT, std::common_comparison_category_t<Args...>>::value, "");
  static_assert(std::is_same<CatT, Expect>::value, "expected different category");
};


// [class.spaceship]p4: The 'common comparison type' U of a possibly-empty list
//   of 'n' types T0, T1, ..., TN, is defined as follows:
int main() {
  using WE = std::weak_equality;
  using SE = std::strong_equality;
  using PO = std::partial_ordering;
  using WO = std::weak_ordering;
  using SO = std::strong_ordering;

  // [class.spaceship]p4.1: If any Ti is not a comparison category tpe, U is void.
  {
    test_cat<void, void>();
    test_cat<void, int*>();
    test_cat<void, SO&>();
    test_cat<void, SO const>();
    test_cat<void, SO*>();
    test_cat<void, SO, void, SO>();
  }

  // [class.spaceship]p4.2: Otherwise, if at least on Ti is
  // std::weak_equality, or at least one Ti is std::strong_equality and at least
  // one Tj is std::partial_ordering or std::weak_ordering, U is std::weak_equality
  {
    test_cat<WE, WE>();
    test_cat<WE, SO, WE, SO>();
    test_cat<WE, SE, SO, PO>();
    test_cat<WE, WO, SO, SE>();
  }

  // [class.spaceship]p4.3: Otherwise, if at least one Ti is std::strong_equality,
  // U is std::strong_equality
  {
    test_cat<SE, SE>();
    test_cat<SE, SO, SE, SO>();
  }

  // [class.spaceship]p4.4: Otherwise, if at least one Ti is std::partial_ordering,
  // U is std::partial_ordering
  {
    test_cat<PO, PO>();
    test_cat<PO, SO, PO, SO>();
    test_cat<PO, WO, PO, SO>();
  }

  // [class.spaceship]p4.5: Otherwise, if at least one Ti is std::weak_ordering,
  // U is std::weak_ordering
  {
    test_cat<WO, WO>();
    test_cat<WO, SO, WO, SO>();
  }

  // [class.spaceship]p4.6: Otherwise, U is std::strong_ordering. [Note: in
  // particular this is the result when n is 0. -- end note]
  {
    test_cat<SO>(); // empty type list
    test_cat<SO, SO>();
    test_cat<SO, SO, SO>();
  }
}
