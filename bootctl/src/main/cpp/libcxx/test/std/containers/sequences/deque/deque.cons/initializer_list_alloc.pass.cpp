//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <deque>

// deque(initializer_list<value_type> il, const Allocator& a = allocator_type());

#include <deque>
#include <cassert>

#include "test_allocator.h"
#include "min_allocator.h"

int main()
{
    {
    std::deque<int, test_allocator<int>> d({3, 4, 5, 6}, test_allocator<int>(3));
    assert(d.get_allocator() == test_allocator<int>(3));
    assert(d.size() == 4);
    assert(d[0] == 3);
    assert(d[1] == 4);
    assert(d[2] == 5);
    assert(d[3] == 6);
    }
    {
    std::deque<int, min_allocator<int>> d({3, 4, 5, 6}, min_allocator<int>());
    assert(d.get_allocator() == min_allocator<int>());
    assert(d.size() == 4);
    assert(d[0] == 3);
    assert(d[1] == 4);
    assert(d[2] == 5);
    assert(d[3] == 6);
    }
}
