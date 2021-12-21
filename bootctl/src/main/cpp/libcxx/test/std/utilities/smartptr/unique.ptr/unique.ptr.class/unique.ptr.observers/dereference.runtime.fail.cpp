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
  std::unique_ptr<int[]> p(new int(3));
  const std::unique_ptr<int[]>& cp = p;
  TEST_IGNORE_NODISCARD (*p);  // expected-error {{indirection requires pointer operand ('std::unique_ptr<int []>' invalid)}}
  TEST_IGNORE_NODISCARD (*cp); // expected-error {{indirection requires pointer operand ('const std::unique_ptr<int []>' invalid)}}
}
