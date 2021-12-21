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

// void clear() noexcept;

#include <set>
#include <cassert>

#include "test_macros.h"
#include "min_allocator.h"

int main()
{
    {
        typedef std::set<int> M;
        typedef int V;
        V ar[] =
        {
            1,
            2,
            3,
            4,
            5,
            6,
            7,
            8
        };
        M m(ar, ar + sizeof(ar)/sizeof(ar[0]));
        assert(m.size() == 8);
        ASSERT_NOEXCEPT(m.clear());
        m.clear();
        assert(m.size() == 0);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::set<int, std::less<int>, min_allocator<int>> M;
        typedef int V;
        V ar[] =
        {
            1,
            2,
            3,
            4,
            5,
            6,
            7,
            8
        };
        M m(ar, ar + sizeof(ar)/sizeof(ar[0]));
        assert(m.size() == 8);
        ASSERT_NOEXCEPT(m.clear());
        m.clear();
        assert(m.size() == 0);
    }
#endif
}
