//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>

// template<ForwardIterator Iter>
//   max_element(Iter first, Iter last);

#include <algorithm>
#include <cassert>

#include "test_iterators.h"

int main() {
  int arr[] = {1, 2, 3};
  const int *b = std::begin(arr), *e = std::end(arr);
  typedef input_iterator<const int*> Iter;
  {
    // expected-error@algorithm:* {{"std::min_element requires a ForwardIterator"}}
    std::min_element(Iter(b), Iter(e));
  }
  {
    // expected-error@algorithm:* {{"std::max_element requires a ForwardIterator"}}
    std::max_element(Iter(b), Iter(e));
  }
  {
    // expected-error@algorithm:* {{"std::minmax_element requires a ForwardIterator"}}
    std::minmax_element(Iter(b), Iter(e));
  }

}
