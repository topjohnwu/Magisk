//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test that <bitset> includes <string> and <iosfwd>

#include <bitset>

template <class> void test_typedef() {}

int main()
{
  { // test for <string>
    std::string s; ((void)s);
  }
  { // test for <iosfwd>
    test_typedef<std::ios>();
    test_typedef<std::wios>();
    test_typedef<std::istream>();
    test_typedef<std::ostream>();
    test_typedef<std::iostream>();
  }
}
