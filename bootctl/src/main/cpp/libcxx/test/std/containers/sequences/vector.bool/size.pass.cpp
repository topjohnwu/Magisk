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

// size_type size() const noexcept;

#include <vector>
#include <cassert>

#include "test_macros.h"
#include "min_allocator.h"

int main()
{
    {
    typedef std::vector<bool> C;
    C c;
    ASSERT_NOEXCEPT(c.size());
    assert(c.size() == 0);
    c.push_back(false);
    assert(c.size() == 1);
    c.push_back(true);
    assert(c.size() == 2);
    c.push_back(false);
    assert(c.size() == 3);
    c.erase(c.begin());
    assert(c.size() == 2);
    c.erase(c.begin());
    assert(c.size() == 1);
    c.erase(c.begin());
    assert(c.size() == 0);
    }
#if TEST_STD_VER >= 11
    {
    typedef std::vector<bool, min_allocator<bool>> C;
    C c;
    ASSERT_NOEXCEPT(c.size());
    assert(c.size() == 0);
    c.push_back(false);
    assert(c.size() == 1);
    c.push_back(true);
    assert(c.size() == 2);
    c.push_back(false);
    assert(c.size() == 3);
    c.erase(c.begin());
    assert(c.size() == 2);
    c.erase(c.begin());
    assert(c.size() == 1);
    c.erase(c.begin());
    assert(c.size() == 0);
    }
#endif
}
