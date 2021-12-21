//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <list>

// iterator insert(const_iterator p, initializer_list<value_type> il);

#include <list>
#include <cassert>

#include "min_allocator.h"

int main()
{
    {
    std::list<int> d(10, 1);
    std::list<int>::iterator i = d.insert(next(d.cbegin(), 2), {3, 4, 5, 6});
    assert(d.size() == 14);
    assert(i == next(d.begin(), 2));
    i = d.begin();
    assert(*i++ == 1);
    assert(*i++ == 1);
    assert(*i++ == 3);
    assert(*i++ == 4);
    assert(*i++ == 5);
    assert(*i++ == 6);
    assert(*i++ == 1);
    assert(*i++ == 1);
    assert(*i++ == 1);
    assert(*i++ == 1);
    assert(*i++ == 1);
    assert(*i++ == 1);
    assert(*i++ == 1);
    assert(*i++ == 1);
    }
    {
    std::list<int, min_allocator<int>> d(10, 1);
    std::list<int, min_allocator<int>>::iterator i = d.insert(next(d.cbegin(), 2), {3, 4, 5, 6});
    assert(d.size() == 14);
    assert(i == next(d.begin(), 2));
    i = d.begin();
    assert(*i++ == 1);
    assert(*i++ == 1);
    assert(*i++ == 3);
    assert(*i++ == 4);
    assert(*i++ == 5);
    assert(*i++ == 6);
    assert(*i++ == 1);
    assert(*i++ == 1);
    assert(*i++ == 1);
    assert(*i++ == 1);
    assert(*i++ == 1);
    assert(*i++ == 1);
    assert(*i++ == 1);
    assert(*i++ == 1);
    }
}
