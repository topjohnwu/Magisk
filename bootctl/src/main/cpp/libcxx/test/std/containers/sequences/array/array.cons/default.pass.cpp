//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <array>

// array();

#include <array>
#include <cassert>

// std::array is explicitly allowed to be initialized with A a = { init-list };.
// Disable the missing braces warning for this reason.
#include "disable_missing_braces_warning.h"

struct NoDefault {
  NoDefault(int) {}
};

int main()
{
    {
        typedef double T;
        typedef std::array<T, 3> C;
        C c;
        assert(c.size() == 3);
    }
    {
        typedef double T;
        typedef std::array<T, 0> C;
        C c;
        assert(c.size() == 0);
    }
    {
      typedef std::array<NoDefault, 0> C;
      C c;
      assert(c.size() == 0);
      C c1 = {};
      assert(c1.size() == 0);
      C c2 = {{}};
      assert(c2.size() == 0);
    }
}
