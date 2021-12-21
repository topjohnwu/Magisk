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

// class multiset

// iterator insert(const_iterator position, value_type&& v);

#include <set>
#include <cassert>

#include "MoveOnly.h"
#include "min_allocator.h"

int main()
{
    {
        typedef std::multiset<MoveOnly> M;
        typedef M::iterator R;
        M m;
        R r = m.insert(m.cend(), M::value_type(2));
        assert(r == m.begin());
        assert(m.size() == 1);
        assert(*r == 2);

        r = m.insert(m.cend(), M::value_type(1));
        assert(r == m.begin());
        assert(m.size() == 2);
        assert(*r == 1);

        r = m.insert(m.cend(), M::value_type(3));
        assert(r == prev(m.end()));
        assert(m.size() == 3);
        assert(*r == 3);

        r = m.insert(m.cend(), M::value_type(3));
        assert(r == prev(m.end()));
        assert(m.size() == 4);
        assert(*r == 3);
    }
    {
        typedef std::multiset<MoveOnly, std::less<MoveOnly>, min_allocator<MoveOnly>> M;
        typedef M::iterator R;
        M m;
        R r = m.insert(m.cend(), M::value_type(2));
        assert(r == m.begin());
        assert(m.size() == 1);
        assert(*r == 2);

        r = m.insert(m.cend(), M::value_type(1));
        assert(r == m.begin());
        assert(m.size() == 2);
        assert(*r == 1);

        r = m.insert(m.cend(), M::value_type(3));
        assert(r == prev(m.end()));
        assert(m.size() == 3);
        assert(*r == 3);

        r = m.insert(m.cend(), M::value_type(3));
        assert(r == prev(m.end()));
        assert(m.size() == 4);
        assert(*r == 3);
    }
}
