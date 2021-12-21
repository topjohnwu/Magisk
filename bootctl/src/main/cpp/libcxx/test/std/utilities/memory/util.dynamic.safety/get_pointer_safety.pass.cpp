//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <memory>

// pointer_safety get_pointer_safety();

// UNSUPPORTED: c++98, c++03

#include <memory>
#include <cassert>


void test_pr26961() {
  std::pointer_safety d;
  d = std::get_pointer_safety();
  assert(d == std::get_pointer_safety());
}

int main()
{
  {
    std::pointer_safety r = std::get_pointer_safety();
    assert(r == std::pointer_safety::relaxed ||
           r == std::pointer_safety::preferred ||
           r == std::pointer_safety::strict);
  }
  {
    test_pr26961();
  }
}
