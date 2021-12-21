//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <list>

// void resize(size_type sz);

#include <list>
#include <cassert>
#include "DefaultOnly.h"
#include "min_allocator.h"

int main()
{
    {
        std::list<int> l(5, 2);
        l.resize(2);
        assert(l.size() == 2);
        assert(std::distance(l.begin(), l.end()) == 2);
        assert(l == std::list<int>(2, 2));
    }
    {
        std::list<int> l(5, 2);
        l.resize(10);
        assert(l.size() == 10);
        assert(std::distance(l.begin(), l.end()) == 10);
        assert(l.front() == 2);
        assert(l.back() == 0);
    }
#ifdef __LIBCPP_MOVE
    {
        std::list<DefaultOnly> l(10);
        l.resize(5);
        assert(l.size() == 5);
        assert(std::distance(l.begin(), l.end()) == 5);
    }
    {
        std::list<DefaultOnly> l(10);
        l.resize(20);
        assert(l.size() == 20);
        assert(std::distance(l.begin(), l.end()) == 20);
    }
#endif  // __LIBCPP_MOVE
#if TEST_STD_VER >= 11
    {
        std::list<int, min_allocator<int>> l(5, 2);
        l.resize(2);
        assert(l.size() == 2);
        assert(std::distance(l.begin(), l.end()) == 2);
        assert((l == std::list<int, min_allocator<int>>(2, 2)));
    }
    {
        std::list<int, min_allocator<int>> l(5, 2);
        l.resize(10);
        assert(l.size() == 10);
        assert(std::distance(l.begin(), l.end()) == 10);
        assert(l.front() == 2);
        assert(l.back() == 0);
    }
#ifdef __LIBCPP_MOVE
    {
        std::list<DefaultOnly, min_allocator<DefaultOnly>> l(10);
        l.resize(5);
        assert(l.size() == 5);
        assert(std::distance(l.begin(), l.end()) == 5);
    }
    {
        std::list<DefaultOnly, min_allocator<DefaultOnly>> l(10);
        l.resize(20);
        assert(l.size() == 20);
        assert(std::distance(l.begin(), l.end()) == 20);
    }
#endif  // __LIBCPP_MOVE
#endif
}
