//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <map>

// class multimap

// iterator insert(const value_type& v);

#include <map>
#include <cassert>

#include "test_macros.h"
#include "min_allocator.h"

template <class Container>
void do_insert_test() {
    typedef Container M;
    typedef typename M::iterator R;
    typedef typename M::value_type VT;
    M m;
    const VT v1(2, 2.5);
    R r = m.insert(v1);
    assert(r == m.begin());
    assert(m.size() == 1);
    assert(r->first == 2);
    assert(r->second == 2.5);

    const VT v2(1, 1.5);
    r = m.insert(v2);
    assert(r == m.begin());
    assert(m.size() == 2);
    assert(r->first == 1);
    assert(r->second == 1.5);

    const VT v3(3, 3.5);
    r = m.insert(v3);
    assert(r == prev(m.end()));
    assert(m.size() == 3);
    assert(r->first == 3);
    assert(r->second == 3.5);

    const VT v4(3, 3.5);
    r = m.insert(v4);
    assert(r == prev(m.end()));
    assert(m.size() == 4);
    assert(r->first == 3);
    assert(r->second == 3.5);
}

int main()
{
    {
        typedef std::multimap<int, double> Container;
        do_insert_test<Container>();
    }
#if TEST_STD_VER >= 11
    {
        typedef std::multimap<int, double, std::less<int>, min_allocator<std::pair<const int, double>>> Container;
        do_insert_test<Container>();
    }
#endif
}
