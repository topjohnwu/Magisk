//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <vector>

// iterator insert(const_iterator p, initializer_list<value_type> il);

#include <vector>
#include <cassert>

#include "min_allocator.h"
#include "asan_testing.h"

int main()
{
    {
    std::vector<int> d(10, 1);
    std::vector<int>::iterator i = d.insert(d.cbegin() + 2, {3, 4, 5, 6});
    assert(d.size() == 14);
    assert(is_contiguous_container_asan_correct(d));
    assert(i == d.begin() + 2);
    assert(d[0] == 1);
    assert(d[1] == 1);
    assert(d[2] == 3);
    assert(d[3] == 4);
    assert(d[4] == 5);
    assert(d[5] == 6);
    assert(d[6] == 1);
    assert(d[7] == 1);
    assert(d[8] == 1);
    assert(d[9] == 1);
    assert(d[10] == 1);
    assert(d[11] == 1);
    assert(d[12] == 1);
    assert(d[13] == 1);
    }
    {
    std::vector<int, min_allocator<int>> d(10, 1);
    std::vector<int, min_allocator<int>>::iterator i = d.insert(d.cbegin() + 2, {3, 4, 5, 6});
    assert(d.size() == 14);
    assert(is_contiguous_container_asan_correct(d));
    assert(i == d.begin() + 2);
    assert(d[0] == 1);
    assert(d[1] == 1);
    assert(d[2] == 3);
    assert(d[3] == 4);
    assert(d[4] == 5);
    assert(d[5] == 6);
    assert(d[6] == 1);
    assert(d[7] == 1);
    assert(d[8] == 1);
    assert(d[9] == 1);
    assert(d[10] == 1);
    assert(d[11] == 1);
    assert(d[12] == 1);
    assert(d[13] == 1);
    }
}
