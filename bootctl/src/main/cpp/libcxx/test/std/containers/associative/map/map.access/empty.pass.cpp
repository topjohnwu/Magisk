//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <map>

// class map

// bool empty() const;

#include <map>
#include <cassert>

#include "min_allocator.h"

int main()
{
    {
    typedef std::map<int, double> M;
    M m;
    assert(m.empty());
    m.insert(M::value_type(1, 1.5));
    assert(!m.empty());
    m.clear();
    assert(m.empty());
    }
#if TEST_STD_VER >= 11
    {
    typedef std::map<int, double, std::less<int>, min_allocator<std::pair<const int, double>>> M;
    M m;
    assert(m.empty());
    m.insert(M::value_type(1, 1.5));
    assert(!m.empty());
    m.clear();
    assert(m.empty());
    }
#endif
}
