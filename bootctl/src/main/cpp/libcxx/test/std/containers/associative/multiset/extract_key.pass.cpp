//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14

// <set>

// class multiset

// node_type extract(key_type const&);

#include <set>
#include "min_allocator.h"
#include "Counter.h"

template <class Container, class KeyTypeIter>
void test(Container& c, KeyTypeIter first, KeyTypeIter last)
{
    size_t sz = c.size();
    assert((size_t)std::distance(first, last) == sz);

    for (KeyTypeIter copy = first; copy != last; ++copy)
    {
        typename Container::node_type t = c.extract(*copy);
        assert(!t.empty());
        --sz;
        assert(t.value() == *copy);
        assert(t.get_allocator() == c.get_allocator());
        assert(sz == c.size());
    }

    assert(c.size() == 0);

    for (KeyTypeIter copy = first; copy != last; ++copy)
    {
        typename Container::node_type t = c.extract(*copy);
        assert(t.empty());
    }
}

int main()
{
    {
        std::multiset<int> m = {1, 2, 3, 4, 5, 6};
        int keys[] = {1, 2, 3, 4, 5, 6};
        test(m, std::begin(keys), std::end(keys));
    }

    {
        std::multiset<Counter<int>> m = {1, 2, 3, 4, 5, 6};
        {
            Counter<int> keys[] = {1, 2, 3, 4, 5, 6};
            assert(Counter_base::gConstructed == 6+6);
            test(m, std::begin(keys), std::end(keys));
        }
        assert(Counter_base::gConstructed == 0);
    }

    {
        using min_alloc_set = std::multiset<int, std::less<int>, min_allocator<int>>;
        min_alloc_set m = {1, 2, 3, 4, 5, 6};
        int keys[] = {1, 2, 3, 4, 5, 6};
        test(m, std::begin(keys), std::end(keys));
    }
}
