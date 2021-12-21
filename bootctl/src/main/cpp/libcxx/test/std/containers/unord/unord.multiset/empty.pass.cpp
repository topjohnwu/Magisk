//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <unordered_set>

// class unordered_multiset

// bool empty() const noexcept;

#include <unordered_set>
#include <cassert>

#include "test_macros.h"
#include "min_allocator.h"

int main()
{
    {
    typedef std::unordered_multiset<int> M;
    M m;
    ASSERT_NOEXCEPT(m.empty());
    assert(m.empty());
    m.insert(M::value_type(1));
    assert(!m.empty());
    m.clear();
    assert(m.empty());
    }
#if TEST_STD_VER >= 11
    {
    typedef std::unordered_multiset<int, std::hash<int>, std::equal_to<int>, min_allocator<int>> M;
    M m;
    ASSERT_NOEXCEPT(m.empty());
    assert(m.empty());
    m.insert(M::value_type(1));
    assert(!m.empty());
    m.clear();
    assert(m.empty());
    }
#endif
}
