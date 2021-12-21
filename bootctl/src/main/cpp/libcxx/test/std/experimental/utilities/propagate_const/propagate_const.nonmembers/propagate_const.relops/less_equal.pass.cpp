//<==----------------------------------------------------------------------<==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//<==----------------------------------------------------------------------<==//

// UNSUPPORTED: c++98, c++03, c++11

// <propagate_const>

// template <class T> constexpr bool operator<=(const propagate_const<T>& x, const propagate_const<T>& y);
// template <class T> constexpr bool operator<=(const T& x, const propagate_const<T>& y);
// template <class T> constexpr bool operator<=(const propagate_const<T>& x, const T& y);

#include <experimental/propagate_const>
#include "propagate_const_helpers.h"
#include <cassert>

using std::experimental::propagate_const;

constexpr bool operator<=(const X &lhs, const X &rhs) {
  return lhs.i_ <= rhs.i_;
}

int main() {
  constexpr X x1_1(1);
  constexpr X x2_1(1);
  constexpr X x3_2(2);

  static_assert(x1_1 <= x2_1, "");
  static_assert(x1_1 <= x3_2, "");
  static_assert(!(x3_2 <= x1_1), "");

  typedef propagate_const<X> P;

  constexpr P p1_1(1);
  constexpr P p2_1(1);
  constexpr P p3_2(2);

  static_assert(p1_1 <= p2_1, "");
  static_assert(p1_1 <= p3_2, "");
  static_assert(!(p3_2 <= p1_1), "");

  static_assert(p1_1 <= x2_1, "");
  static_assert(p1_1 <= x3_2, "");
  static_assert(!(p3_2 <= x1_1), "");

  static_assert(x1_1 <= p2_1, "");
  static_assert(x1_1 <= p3_2, "");
  static_assert(!(x3_2 <= p1_1), "");

}
