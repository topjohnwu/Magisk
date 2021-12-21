//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <vector>

// void pop_back();

#include <vector>
#include <cassert>

#include "test_macros.h"
#include "test_allocator.h"
#include "min_allocator.h"


int main()
{
    {
        std::vector<int> c;
        c.push_back(1);
        assert(c.size() == 1);
        c.pop_back();
        assert(c.size() == 0);

    }
#if TEST_STD_VER >= 11
    {
        std::vector<int, min_allocator<int>> c;
        c.push_back(1);
        assert(c.size() == 1);
        c.pop_back();
        assert(c.size() == 0);
    }
#endif
}
