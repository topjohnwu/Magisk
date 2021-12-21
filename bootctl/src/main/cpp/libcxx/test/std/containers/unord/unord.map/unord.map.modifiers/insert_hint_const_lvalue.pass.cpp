//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <unordered_map>

// template <class Key, class T, class Hash = hash<Key>, class Pred = equal_to<Key>,
//           class Alloc = allocator<pair<const Key, T>>>
// class unordered_map

// iterator insert(const_iterator p, const value_type& x);

#if _LIBCPP_DEBUG >= 1
#define _LIBCPP_ASSERT(x, m) ((x) ? (void)0 : std::exit(0))
#endif

#include <unordered_map>
#include <cassert>

#include "min_allocator.h"

template<class Container>
void do_insert_hint_const_lvalue_test()
{
    typedef Container C;
    typedef typename C::iterator R;
    typedef typename C::value_type VT;
    C c;
    typename C::const_iterator e = c.end();
    const VT v1(3.5, 3);
    R r = c.insert(e, v1);
    assert(c.size() == 1);
    assert(r->first == 3.5);
    assert(r->second == 3);

    const VT v2(3.5, 4);
    r = c.insert(c.end(), v2);
    assert(c.size() == 1);
    assert(r->first == 3.5);
    assert(r->second == 3);

    const VT v3(4.5, 4);
    r = c.insert(c.end(), v3);
    assert(c.size() == 2);
    assert(r->first == 4.5);
    assert(r->second == 4);

    const VT v4(5.5, 4);
    r = c.insert(c.end(), v4);
    assert(c.size() == 3);
    assert(r->first == 5.5);
    assert(r->second == 4);
}

int main()
{
    do_insert_hint_const_lvalue_test<std::unordered_map<double, int> >();
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_map<double, int, std::hash<double>, std::equal_to<double>,
                            min_allocator<std::pair<const double, int>>> C;

        do_insert_hint_const_lvalue_test<C>();
    }
#endif
#if _LIBCPP_DEBUG >= 1
    {
        typedef std::unordered_map<double, int> C;
        typedef C::iterator R;
        typedef C::value_type P;
        C c;
        C c2;
        C::const_iterator e = c2.end();
        P v(3.5, 3);
        R r = c.insert(e, v);
        assert(false);
    }
#endif
}
