//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <vector>
// vector<bool>

// void resize(size_type sz, const value_type& x);

#include <vector>
#include <cassert>

#include "min_allocator.h"

int main()
{
    {
        std::vector<bool> v(100);
        v.resize(50, 1);
        assert(v.size() == 50);
        assert(v.capacity() >= 100);
        assert(v == std::vector<bool>(50));
        v.resize(200, 1);
        assert(v.size() == 200);
        assert(v.capacity() >= 200);
        for (unsigned i = 0; i < 50; ++i)
            assert(v[i] == 0);
        for (unsigned i = 50; i < 200; ++i)
            assert(v[i] == 1);
    }
#if TEST_STD_VER >= 11
    {
        std::vector<bool, min_allocator<bool>> v(100);
        v.resize(50, 1);
        assert(v.size() == 50);
        assert(v.capacity() >= 100);
        assert((v == std::vector<bool, min_allocator<bool>>(50)));
        v.resize(200, 1);
        assert(v.size() == 200);
        assert(v.capacity() >= 200);
        for (unsigned i = 0; i < 50; ++i)
            assert(v[i] == 0);
        for (unsigned i = 50; i < 200; ++i)
            assert(v[i] == 1);
    }
#endif
}
