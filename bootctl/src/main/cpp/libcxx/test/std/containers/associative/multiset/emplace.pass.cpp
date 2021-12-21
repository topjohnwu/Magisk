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

// template <class... Args>
//   iterator emplace(Args&&... args);

#include <set>
#include <cassert>

#include "../../Emplaceable.h"
#include "DefaultOnly.h"
#include "min_allocator.h"

int main()
{
    {
        typedef std::multiset<DefaultOnly> M;
        typedef M::iterator R;
        M m;
        assert(DefaultOnly::count == 0);
        R r = m.emplace();
        assert(r == m.begin());
        assert(m.size() == 1);
        assert(*m.begin() == DefaultOnly());
        assert(DefaultOnly::count == 1);

        r = m.emplace();
        assert(r == next(m.begin()));
        assert(m.size() == 2);
        assert(*m.begin() == DefaultOnly());
        assert(DefaultOnly::count == 2);
    }
    assert(DefaultOnly::count == 0);
    {
        typedef std::multiset<Emplaceable> M;
        typedef M::iterator R;
        M m;
        R r = m.emplace();
        assert(r == m.begin());
        assert(m.size() == 1);
        assert(*m.begin() == Emplaceable());
        r = m.emplace(2, 3.5);
        assert(r == next(m.begin()));
        assert(m.size() == 2);
        assert(*r == Emplaceable(2, 3.5));
        r = m.emplace(2, 3.5);
        assert(r == next(m.begin(), 2));
        assert(m.size() == 3);
        assert(*r == Emplaceable(2, 3.5));
    }
    {
        typedef std::multiset<int> M;
        typedef M::iterator R;
        M m;
        R r = m.emplace(M::value_type(2));
        assert(r == m.begin());
        assert(m.size() == 1);
        assert(*r == 2);
    }
    {
        typedef std::multiset<int, std::less<int>, min_allocator<int>> M;
        typedef M::iterator R;
        M m;
        R r = m.emplace(M::value_type(2));
        assert(r == m.begin());
        assert(m.size() == 1);
        assert(*r == 2);
    }
}
