//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <vector>

// class vector

// bool empty() const noexcept;

#include <vector>
#include <cassert>

#include "test_macros.h"
#include "min_allocator.h"

int main()
{
    {
    typedef std::vector<bool> C;
    C c;
    ASSERT_NOEXCEPT(c.empty());
    assert(c.empty());
    c.push_back(false);
    assert(!c.empty());
    c.clear();
    assert(c.empty());
    }
#if TEST_STD_VER >= 11
    {
    typedef std::vector<bool, min_allocator<bool>> C;
    C c;
    ASSERT_NOEXCEPT(c.empty());
    assert(c.empty());
    c.push_back(false);
    assert(!c.empty());
    c.clear();
    assert(c.empty());
    }
#endif
}
