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

// iterator insert(const_iterator position, const value_type& v);

#include <set>
#include <cassert>

#include "min_allocator.h"

int main()
{
    {
        typedef std::set<int> M;
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
        assert(m.size() == 3);
        assert(*r == 3);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::set<int, std::less<int>, min_allocator<int>> M;
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
        assert(m.size() == 3);
        assert(*r == 3);
    }
#endif
}
