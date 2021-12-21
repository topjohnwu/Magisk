//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <unordered_set>

// class unordered_set

// size_type size() const noexcept;

#include <unordered_set>
#include <cassert>

#include "test_macros.h"
#include "min_allocator.h"

int main()
{
    {
    typedef std::unordered_set<int> M;
    M m;
    ASSERT_NOEXCEPT(m.size());
    assert(m.size() == 0);
    m.insert(M::value_type(2));
    assert(m.size() == 1);
    m.insert(M::value_type(1));
    assert(m.size() == 2);
    m.insert(M::value_type(3));
    assert(m.size() == 3);
    m.erase(m.begin());
    assert(m.size() == 2);
    m.erase(m.begin());
    assert(m.size() == 1);
    m.erase(m.begin());
    assert(m.size() == 0);
    }
#if TEST_STD_VER >= 11
    {
    typedef std::unordered_set<int, std::hash<int>, std::equal_to<int>, min_allocator<int>> M;
    M m;
    ASSERT_NOEXCEPT(m.size());
    assert(m.size() == 0);
    m.insert(M::value_type(2));
    assert(m.size() == 1);
    m.insert(M::value_type(1));
    assert(m.size() == 2);
    m.insert(M::value_type(3));
    assert(m.size() == 3);
    m.erase(m.begin());
    assert(m.size() == 2);
    m.erase(m.begin());
    assert(m.size() == 1);
    m.erase(m.begin());
    assert(m.size() == 0);
    }
#endif
}
