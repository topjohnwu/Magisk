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

// pair<iterator, bool> insert(const value_type& v);

#include <set>
#include <cassert>

#include "min_allocator.h"

template<class Container>
void do_insert_cv_test()
{
    typedef Container M;
    typedef std::pair<typename M::iterator, bool> R;
    typedef typename M::value_type VT;
    M m;

    const VT v1(2);
    R r = m.insert(v1);
    assert(r.second);
    assert(r.first == m.begin());
    assert(m.size() == 1);
    assert(*r.first == 2);

    const VT v2(1);
    r = m.insert(v2);
    assert(r.second);
    assert(r.first == m.begin());
    assert(m.size() == 2);
    assert(*r.first == 1);

    const VT v3(3);
    r = m.insert(v3);
    assert(r.second);
    assert(r.first == prev(m.end()));
    assert(m.size() == 3);
    assert(*r.first == 3);

    r = m.insert(v3);
    assert(!r.second);
    assert(r.first == prev(m.end()));
    assert(m.size() == 3);
    assert(*r.first == 3);
}

int main()
{
    do_insert_cv_test<std::set<int> >();
#if TEST_STD_VER >= 11
    {
        typedef std::set<int, std::less<int>, min_allocator<int>> M;
        do_insert_cv_test<M>();
    }
#endif
}
