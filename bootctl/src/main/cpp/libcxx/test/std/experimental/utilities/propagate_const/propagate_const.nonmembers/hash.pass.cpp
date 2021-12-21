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

// template <class T> struct hash<experimental::fundamentals_v2::propagate_const<T>>;

#include <experimental/propagate_const>
#include "propagate_const_helpers.h"
#include <cassert>

using std::experimental::propagate_const;

namespace std {
template <> struct hash<X>
{
  typedef X first_argument_type;

  size_t operator()(const first_argument_type&) const
  {
    return 99;
  }

};
} // namespace std

int main() {

  typedef propagate_const<X> P;

  P p(1);

  auto h = std::hash<P>();

  assert(h(p)==99);
}
