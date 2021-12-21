//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <unordered_set>

// template <class Value, class Hash = hash<Value>, class Pred = equal_to<Value>,
//           class Alloc = allocator<Value>>
// class unordered_set

// pair<iterator, bool> insert(value_type&& x);

#include <unordered_set>
#include <cassert>

#include "test_macros.h"
#include "MoveOnly.h"
#include "min_allocator.h"

int main()
{
    {
        typedef std::unordered_set<double> C;
        typedef std::pair<C::iterator, bool> R;
        typedef double P;
        C c;
        R r = c.insert(P(3.5));
        assert(c.size() == 1);
        assert(*r.first == 3.5);
        assert(r.second);

        r = c.insert(P(3.5));
        assert(c.size() == 1);
        assert(*r.first == 3.5);
        assert(!r.second);

        r = c.insert(P(4.5));
        assert(c.size() == 2);
        assert(*r.first == 4.5);
        assert(r.second);

        r = c.insert(P(5.5));
        assert(c.size() == 3);
        assert(*r.first == 5.5);
        assert(r.second);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_set<MoveOnly> C;
        typedef std::pair<C::iterator, bool> R;
        typedef MoveOnly P;
        C c;
        R r = c.insert(P(3));
        assert(c.size() == 1);
        assert(*r.first == 3);
        assert(r.second);

        r = c.insert(P(3));
        assert(c.size() == 1);
        assert(*r.first == 3);
        assert(!r.second);

        r = c.insert(P(4));
        assert(c.size() == 2);
        assert(*r.first == 4);
        assert(r.second);

        r = c.insert(P(5));
        assert(c.size() == 3);
        assert(*r.first == 5);
        assert(r.second);
    }
    {
        typedef std::unordered_set<double, std::hash<double>,
                                std::equal_to<double>, min_allocator<double>> C;
        typedef std::pair<C::iterator, bool> R;
        typedef double P;
        C c;
        R r = c.insert(P(3.5));
        assert(c.size() == 1);
        assert(*r.first == 3.5);
        assert(r.second);

        r = c.insert(P(3.5));
        assert(c.size() == 1);
        assert(*r.first == 3.5);
        assert(!r.second);

        r = c.insert(P(4.5));
        assert(c.size() == 2);
        assert(*r.first == 4.5);
        assert(r.second);

        r = c.insert(P(5.5));
        assert(c.size() == 3);
        assert(*r.first == 5.5);
        assert(r.second);
    }
    {
        typedef std::unordered_set<MoveOnly, std::hash<MoveOnly>,
                            std::equal_to<MoveOnly>, min_allocator<MoveOnly>> C;
        typedef std::pair<C::iterator, bool> R;
        typedef MoveOnly P;
        C c;
        R r = c.insert(P(3));
        assert(c.size() == 1);
        assert(*r.first == 3);
        assert(r.second);

        r = c.insert(P(3));
        assert(c.size() == 1);
        assert(*r.first == 3);
        assert(!r.second);

        r = c.insert(P(4));
        assert(c.size() == 2);
        assert(*r.first == 4);
        assert(r.second);

        r = c.insert(P(5));
        assert(c.size() == 3);
        assert(*r.first == 5);
        assert(r.second);
    }
#endif // TEST_STD_VER >= 11
}
