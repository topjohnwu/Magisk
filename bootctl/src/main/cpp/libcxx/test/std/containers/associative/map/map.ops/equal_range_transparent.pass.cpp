//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11

// <map>

// class map

// template<typename K>
//         pair<iterator,iterator>             equal_range(const K& x); // C++14
// template<typename K>
//         pair<const_iterator,const_iterator> equal_range(const K& x) const;
//         // C++14

#include <cassert>
#include <map>
#include <utility>

#include "min_allocator.h"
#include "private_constructor.hpp"
#include "test_macros.h"

struct Comp {
  using is_transparent = void;

  bool operator()(const std::pair<int, int> &lhs,
                  const std::pair<int, int> &rhs) const {
    return lhs < rhs;
  }

  bool operator()(const std::pair<int, int> &lhs, int rhs) const {
    return lhs.first < rhs;
  }

  bool operator()(int lhs, const std::pair<int, int> &rhs) const {
    return lhs < rhs.first;
  }
};

int main() {
  std::map<std::pair<int, int>, int, Comp> s{
      {{2, 1}, 1}, {{1, 2}, 2}, {{1, 3}, 3}, {{1, 4}, 4}, {{2, 2}, 5}};

  auto er = s.equal_range(1);
  long nels = 0;

  for (auto it = er.first; it != er.second; it++) {
    assert(it->first.first == 1);
    nels++;
  }

  assert(nels == 3);
}
