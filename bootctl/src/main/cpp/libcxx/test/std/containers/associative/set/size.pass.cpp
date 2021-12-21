//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <set>

// class set

// size_type size() const;

#include <set>
#include <cassert>

#include "min_allocator.h"

int main()
{
    {
    typedef std::set<int> M;
    M m;
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
    typedef std::set<int, std::less<int>, min_allocator<int>> M;
    M m;
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
