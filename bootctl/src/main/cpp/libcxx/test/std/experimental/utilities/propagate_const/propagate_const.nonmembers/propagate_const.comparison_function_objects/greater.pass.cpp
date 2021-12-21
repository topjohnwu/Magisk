//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11

// <propagate_const>

// template <class T> struct greater<experimental::fundamentals_v2::propagate_const<T>>;

#include <experimental/propagate_const>
#include "propagate_const_helpers.h"
#include <cassert>

using std::experimental::propagate_const;

constexpr bool operator>(const X &x1, const X &x2) { return x1.i_ > x2.i_; }

int main() {

  typedef propagate_const<X> P;

  P p1_1(1);
  P p2_1(1);
  P p3_2(2);

  auto c = std::greater<P>();

  assert(!c(p1_1,p2_1));
  assert(!c(p2_1,p1_1));
  assert(!c(p1_1,p3_2));
  assert(c(p3_2,p1_1));
}
