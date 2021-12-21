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

// template <class... Args>
//   iterator emplace_hint(const_iterator position, Args&&... args);

#include <set>
#include <cassert>

#include "../../Emplaceable.h"
#include "DefaultOnly.h"
#include "min_allocator.h"

int main()
{
    {
        typedef std::set<DefaultOnly> M;
        typedef M::iterator R;
        M m;
        assert(DefaultOnly::count == 0);
        R r = m.emplace_hint(m.cend());
        assert(r == m.begin());
        assert(m.size() == 1);
        assert(*m.begin() == DefaultOnly());
        assert(DefaultOnly::count == 1);

        r = m.emplace_hint(m.cbegin());
        assert(r == m.begin());
        assert(m.size() == 1);
        assert(*m.begin() == DefaultOnly());
        assert(DefaultOnly::count == 1);
    }
    assert(DefaultOnly::count == 0);
    {
        typedef std::set<Emplaceable> M;
        typedef M::iterator R;
        M m;
        R r = m.emplace_hint(m.cend());
        assert(r == m.begin());
        assert(m.size() == 1);
        assert(*m.begin() == Emplaceable());
        r = m.emplace_hint(m.cend(), 2, 3.5);
        assert(r == next(m.begin()));
        assert(m.size() == 2);
        assert(*r == Emplaceable(2, 3.5));
        r = m.emplace_hint(m.cbegin(), 2, 3.5);
        assert(r == next(m.begin()));
        assert(m.size() == 2);
        assert(*r == Emplaceable(2, 3.5));
    }
    {
        typedef std::set<int> M;
        typedef M::iterator R;
        M m;
        R r = m.emplace_hint(m.cend(), M::value_type(2));
        assert(r == m.begin());
        assert(m.size() == 1);
        assert(*r == 2);
    }
    {
        typedef std::set<int, std::less<int>, min_allocator<int>> M;
        typedef M::iterator R;
        M m;
        R r = m.emplace_hint(m.cend(), M::value_type(2));
        assert(r == m.begin());
        assert(m.size() == 1);
        assert(*r == 2);
    }
}
