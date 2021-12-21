//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <list>

// explicit list(const Alloc& = Alloc());

#include <list>
#include <cassert>
#include "DefaultOnly.h"
#include "min_allocator.h"

int main()
{
    {
        std::list<int> l;
        assert(l.size() == 0);
        assert(std::distance(l.begin(), l.end()) == 0);
    }
    {
        std::list<DefaultOnly> l;
        assert(l.size() == 0);
        assert(std::distance(l.begin(), l.end()) == 0);
    }
    {
        std::list<int> l((std::allocator<int>()));
        assert(l.size() == 0);
        assert(std::distance(l.begin(), l.end()) == 0);
    }
#if TEST_STD_VER >= 11
    {
        std::list<int, min_allocator<int>> l;
        assert(l.size() == 0);
        assert(std::distance(l.begin(), l.end()) == 0);
    }
    {
        std::list<DefaultOnly, min_allocator<DefaultOnly>> l;
        assert(l.size() == 0);
        assert(std::distance(l.begin(), l.end()) == 0);
    }
    {
        std::list<int, min_allocator<int>> l((min_allocator<int>()));
        assert(l.size() == 0);
        assert(std::distance(l.begin(), l.end()) == 0);
    }
    {
        std::list<int> l = {};
        assert(l.size() == 0);
        assert(std::distance(l.begin(), l.end()) == 0);
    }
    {
        std::list<int, explicit_allocator<int>> l;
        assert(l.size() == 0);
        assert(std::distance(l.begin(), l.end()) == 0);
    }
    {
        std::list<int, explicit_allocator<int>> l((explicit_allocator<int>()));
        assert(l.size() == 0);
        assert(std::distance(l.begin(), l.end()) == 0);
    }
#endif
}
