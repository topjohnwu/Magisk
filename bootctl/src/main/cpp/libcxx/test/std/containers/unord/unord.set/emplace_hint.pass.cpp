//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <unordered_set>

// template <class Value, class Hash = hash<Value>, class Pred = equal_to<Value>,
//           class Alloc = allocator<Value>>
// class unordered_set

// template <class... Args>
//     iterator emplace_hint(const_iterator p, Args&&... args);


#include <unordered_set>
#include <cassert>

#include "../../Emplaceable.h"
#include "min_allocator.h"

int main()
{
    {
        typedef std::unordered_set<Emplaceable> C;
        typedef C::iterator R;
        C c;
        C::const_iterator e = c.end();
        R r = c.emplace_hint(e);
        assert(c.size() == 1);
        assert(*r == Emplaceable());

        r = c.emplace_hint(e, Emplaceable(5, 6));
        assert(c.size() == 2);
        assert(*r == Emplaceable(5, 6));

        r = c.emplace_hint(r, 5, 6);
        assert(c.size() == 2);
        assert(*r == Emplaceable(5, 6));
    }
    {
        typedef std::unordered_set<Emplaceable, std::hash<Emplaceable>,
                      std::equal_to<Emplaceable>, min_allocator<Emplaceable>> C;
        typedef C::iterator R;
        C c;
        C::const_iterator e = c.end();
        R r = c.emplace_hint(e);
        assert(c.size() == 1);
        assert(*r == Emplaceable());

        r = c.emplace_hint(e, Emplaceable(5, 6));
        assert(c.size() == 2);
        assert(*r == Emplaceable(5, 6));

        r = c.emplace_hint(r, 5, 6);
        assert(c.size() == 2);
        assert(*r == Emplaceable(5, 6));
    }
}
