//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <deque>

// void clear() noexcept;

#include <deque>
#include <cassert>

#include "test_macros.h"
#include "../../../NotConstructible.h"
#include "min_allocator.h"

int main()
{
    {
        typedef NotConstructible T;
        typedef std::deque<T> C;
        C c;
        ASSERT_NOEXCEPT(c.clear());
        c.clear();
        assert(distance(c.begin(), c.end()) == 0);
    }
    {
        typedef int T;
        typedef std::deque<T> C;
        const T t[] = {0, 1, 2, 3, 4};
        C c(std::begin(t), std::end(t));

        ASSERT_NOEXCEPT(c.clear());
        c.clear();
        assert(distance(c.begin(), c.end()) == 0);

        c.clear();
        assert(distance(c.begin(), c.end()) == 0);
    }
#if TEST_STD_VER >= 11
    {
        typedef NotConstructible T;
        typedef std::deque<T, min_allocator<T>> C;
        C c;
        ASSERT_NOEXCEPT(c.clear());
        c.clear();
        assert(distance(c.begin(), c.end()) == 0);
    }
    {
        typedef int T;
        typedef std::deque<T, min_allocator<T>> C;
        const T t[] = {0, 1, 2, 3, 4};
        C c(std::begin(t), std::end(t));

        ASSERT_NOEXCEPT(c.clear());
        c.clear();
        assert(distance(c.begin(), c.end()) == 0);

        c.clear();
        assert(distance(c.begin(), c.end()) == 0);
    }
#endif
}
