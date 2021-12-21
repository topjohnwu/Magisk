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

// bool empty() const noexcept;

#include <deque>
#include <cassert>

#include "test_macros.h"
#include "min_allocator.h"

int main()
{
    {
    typedef std::deque<int> C;
    C c;
    ASSERT_NOEXCEPT(c.empty());
    assert(c.empty());
    c.push_back(C::value_type(1));
    assert(!c.empty());
    c.clear();
    assert(c.empty());
    }
#if TEST_STD_VER >= 11
    {
    typedef std::deque<int, min_allocator<int>> C;
    C c;
    ASSERT_NOEXCEPT(c.empty());
    assert(c.empty());
    c.push_back(C::value_type(1));
    assert(!c.empty());
    c.clear();
    assert(c.empty());
    }
#endif
}
