//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <set>

// class set

// void insert(initializer_list<value_type> il);

#include <set>
#include <cassert>
#include <cstddef>

#include "min_allocator.h"

int main()
{
    {
    typedef std::set<int> C;
    typedef C::value_type V;
    C m = {10, 8};
    m.insert({1, 2, 3, 4, 5, 6});
    assert(m.size() == 8);
    assert(static_cast<std::size_t>(distance(m.begin(), m.end())) == m.size());
    C::const_iterator i = m.cbegin();
    assert(*i == V(1));
    assert(*++i == V(2));
    assert(*++i == V(3));
    assert(*++i == V(4));
    assert(*++i == V(5));
    assert(*++i == V(6));
    assert(*++i == V(8));
    assert(*++i == V(10));
    }
    {
    typedef std::set<int, std::less<int>, min_allocator<int>> C;
    typedef C::value_type V;
    C m = {10, 8};
    m.insert({1, 2, 3, 4, 5, 6});
    assert(m.size() == 8);
    assert(static_cast<std::size_t>(distance(m.begin(), m.end())) == m.size());
    C::const_iterator i = m.cbegin();
    assert(*i == V(1));
    assert(*++i == V(2));
    assert(*++i == V(3));
    assert(*++i == V(4));
    assert(*++i == V(5));
    assert(*++i == V(6));
    assert(*++i == V(8));
    assert(*++i == V(10));
    }
}
