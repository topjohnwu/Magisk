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

// template <class T> constexpr void swap(propagate_const<T>& x, propagate_const<T>& y);

#include <experimental/propagate_const>
#include "propagate_const_helpers.h"
#include <cassert>

using std::experimental::propagate_const;

bool swap_called = false;
void swap(X &, X &) { swap_called = true; }

int main() {
  typedef propagate_const<X> P;
  P p1(1);
  P p2(2);
  swap(p1, p2);
  assert(swap_called);
}
