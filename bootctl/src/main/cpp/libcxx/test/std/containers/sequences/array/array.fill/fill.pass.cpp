//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <array>

// void fill(const T& u);

#include <array>
#include <cassert>

// std::array is explicitly allowed to be initialized with A a = { init-list };.
// Disable the missing braces warning for this reason.
#include "disable_missing_braces_warning.h"

int main()
{
    {
        typedef double T;
        typedef std::array<T, 3> C;
        C c = {1, 2, 3.5};
        c.fill(5.5);
        assert(c.size() == 3);
        assert(c[0] == 5.5);
        assert(c[1] == 5.5);
        assert(c[2] == 5.5);
    }
    {
        typedef double T;
        typedef std::array<T, 0> C;
        C c = {};
        c.fill(5.5);
        assert(c.size() == 0);
    }
}
