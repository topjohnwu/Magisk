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

// pair<iterator, bool> insert(value_type&& v);

#include <set>
#include <cassert>

#include "MoveOnly.h"
#include "min_allocator.h"

int main()
{
    {
        typedef std::set<MoveOnly> M;
        typedef std::pair<M::iterator, bool> R;
        M m;
        R r = m.insert(M::value_type(2));
        assert(r.second);
        assert(r.first == m.begin());
        assert(m.size() == 1);
        assert(*r.first == 2);

        r = m.insert(M::value_type(1));
        assert(r.second);
        assert(r.first == m.begin());
        assert(m.size() == 2);
        assert(*r.first == 1);

        r = m.insert(M::value_type(3));
        assert(r.second);
        assert(r.first == prev(m.end()));
        assert(m.size() == 3);
        assert(*r.first == 3);

        r = m.insert(M::value_type(3));
        assert(!r.second);
        assert(r.first == prev(m.end()));
        assert(m.size() == 3);
        assert(*r.first == 3);
    }
    {
        typedef std::set<MoveOnly, std::less<MoveOnly>, min_allocator<MoveOnly>> M;
        typedef std::pair<M::iterator, bool> R;
        M m;
        R r = m.insert(M::value_type(2));
        assert(r.second);
        assert(r.first == m.begin());
        assert(m.size() == 1);
        assert(*r.first == 2);

        r = m.insert(M::value_type(1));
        assert(r.second);
        assert(r.first == m.begin());
        assert(m.size() == 2);
        assert(*r.first == 1);

        r = m.insert(M::value_type(3));
        assert(r.second);
        assert(r.first == prev(m.end()));
        assert(m.size() == 3);
        assert(*r.first == 3);

        r = m.insert(M::value_type(3));
        assert(!r.second);
        assert(r.first == prev(m.end()));
        assert(m.size() == 3);
        assert(*r.first == 3);
    }
}
