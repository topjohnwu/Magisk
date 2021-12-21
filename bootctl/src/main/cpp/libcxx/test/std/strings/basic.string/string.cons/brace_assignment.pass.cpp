//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <string>

// basic_string<charT,traits,Allocator>&
//   operator=(basic_string<charT,traits,Allocator>&& str);

#include <string>
#include <cassert>

#include "test_macros.h"

int main()
{
  // Test that assignment from {} and {ptr, len} are allowed and are not
  // ambiguous.
  {
    std::string s = "hello world";
    s = {};
    assert(s.empty());
  }
  {
    std::string s = "hello world";
    s = {"abc", 2};
    assert(s == "ab");
  }
}
