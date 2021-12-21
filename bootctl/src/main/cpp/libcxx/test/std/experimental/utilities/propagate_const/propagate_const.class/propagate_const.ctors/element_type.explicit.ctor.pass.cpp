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

// template <class U> propagate_const(U&&);

#include <experimental/propagate_const>
#include "propagate_const_helpers.h"
#include <type_traits>

using std::experimental::propagate_const;

typedef propagate_const<ExplicitX> P;

int main() {
  static_assert(!std::is_convertible<P, int>::value, "");
  static_assert(std::is_constructible<P, int>::value, "");
}

