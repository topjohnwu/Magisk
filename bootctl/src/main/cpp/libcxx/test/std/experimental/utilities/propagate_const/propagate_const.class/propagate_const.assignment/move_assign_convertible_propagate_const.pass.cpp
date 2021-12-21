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

// template <class U> constexpr propagate_const& operator=(propagate_const<_Up>&& pu);

#include <experimental/propagate_const>
#include "propagate_const_helpers.h"
#include <cassert>

using std::experimental::propagate_const;

int main() {

  typedef propagate_const<X> PX;
  typedef propagate_const<MoveConstructibleFromX> PY;

  PX px2(2);
  PY py1(1);

  py1=std::move(px2);

  assert(*py1==2);
}
