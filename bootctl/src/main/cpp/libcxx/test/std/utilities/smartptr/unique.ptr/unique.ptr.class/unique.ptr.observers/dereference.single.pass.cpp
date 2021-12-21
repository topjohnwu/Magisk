//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <memory>

// unique_ptr

// test op*()

#include <memory>
#include <cassert>

int main() {
  std::unique_ptr<int> p(new int(3));
  assert(*p == 3);
}
