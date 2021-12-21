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

// void resize(size_type sz);

#include <vector>
#include <cassert>

#include "min_allocator.h"

int main()
{
    {
        std::vector<bool> v(100);
        v.resize(50);
        assert(v.size() == 50);
        assert(v.capacity() >= 100);
        v.resize(200);
        assert(v.size() == 200);
        assert(v.capacity() >= 200);
        v.reserve(400);
        v.resize(300);  // check the case when resizing and we already have room
        assert(v.size() == 300);
        assert(v.capacity() >= 400);
    }
#if TEST_STD_VER >= 11
    {
        std::vector<bool, min_allocator<bool>> v(100);
        v.resize(50);
        assert(v.size() == 50);
        assert(v.capacity() >= 100);
        v.resize(200);
        assert(v.size() == 200);
        assert(v.capacity() >= 200);
        v.reserve(400);
        v.resize(300);  // check the case when resizing and we already have room
        assert(v.size() == 300);
        assert(v.capacity() >= 400);
    }
#endif
}
