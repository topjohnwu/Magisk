//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <array>

// class array

// bool max_size() const noexcept;

#include <array>
#include <cassert>

#include "test_macros.h"
#include "min_allocator.h"

int main()
{
    {
    typedef std::array<int, 2> C;
    C c;
    ASSERT_NOEXCEPT(c.max_size());
    assert(c.max_size() == 2);
    }
    {
    typedef std::array<int, 0> C;
    C c;
    ASSERT_NOEXCEPT(c.max_size());
    assert(c.max_size() == 0);
    }
}
