//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <unordered_map>

// template <class Key, class T, class Hash = hash<Key>, class Pred = equal_to<Key>,
//           class Alloc = allocator<pair<const Key, T>>>
// class unordered_map

// template <class... Args>
//     iterator emplace_hint(const_iterator p, Args&&... args);


#include <unordered_map>
#include <cassert>

#include "../../../Emplaceable.h"
#include "min_allocator.h"

int main()
{
    {
        typedef std::unordered_map<int, Emplaceable> C;
        typedef C::iterator R;
        C c;
        C::const_iterator e = c.end();
        R r = c.emplace_hint(e, std::piecewise_construct, std::forward_as_tuple(3),
                                                          std::forward_as_tuple());
        assert(c.size() == 1);
        assert(r->first == 3);
        assert(r->second == Emplaceable());

        r = c.emplace_hint(c.end(), std::pair<const int, Emplaceable>(4, Emplaceable(5, 6)));
        assert(c.size() == 2);
        assert(r->first == 4);
        assert(r->second == Emplaceable(5, 6));

        r = c.emplace_hint(c.end(), std::piecewise_construct, std::forward_as_tuple(5),
                                                       std::forward_as_tuple(6, 7));
        assert(c.size() == 3);
        assert(r->first == 5);
        assert(r->second == Emplaceable(6, 7));
    }
    {
        typedef std::unordered_map<int, Emplaceable, std::hash<int>, std::equal_to<int>,
                            min_allocator<std::pair<const int, Emplaceable>>> C;
        typedef C::iterator R;
        C c;
        C::const_iterator e = c.end();
        R r = c.emplace_hint(e, std::piecewise_construct, std::forward_as_tuple(3),
                                                          std::forward_as_tuple());
        assert(c.size() == 1);
        assert(r->first == 3);
        assert(r->second == Emplaceable());

        r = c.emplace_hint(c.end(), std::pair<const int, Emplaceable>(4, Emplaceable(5, 6)));
        assert(c.size() == 2);
        assert(r->first == 4);
        assert(r->second == Emplaceable(5, 6));

        r = c.emplace_hint(c.end(), std::piecewise_construct, std::forward_as_tuple(5),
                                                       std::forward_as_tuple(6, 7));
        assert(c.size() == 3);
        assert(r->first == 5);
        assert(r->second == Emplaceable(6, 7));
    }
}
