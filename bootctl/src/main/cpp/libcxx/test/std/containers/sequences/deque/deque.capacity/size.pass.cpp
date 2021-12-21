//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <deque>

// class deque

// size_type size() const noexcept;

#include <deque>
#include <cassert>

#include "test_macros.h"
#include "min_allocator.h"

int main()
{
    {
    typedef std::deque<int> C;
    C c;
    ASSERT_NOEXCEPT(c.size());
    assert(c.size() == 0);
    c.push_back(C::value_type(2));
    assert(c.size() == 1);
    c.push_back(C::value_type(1));
    assert(c.size() == 2);
    c.push_back(C::value_type(3));
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
    typedef std::deque<int, min_allocator<int>> C;
    C c;
    ASSERT_NOEXCEPT(c.size());
    assert(c.size() == 0);
    c.push_back(C::value_type(2));
    assert(c.size() == 1);
    c.push_back(C::value_type(1));
    assert(c.size() == 2);
    c.push_back(C::value_type(3));
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
