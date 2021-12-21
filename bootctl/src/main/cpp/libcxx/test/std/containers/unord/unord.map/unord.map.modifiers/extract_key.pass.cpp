//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14

// <unordered_map>

// class unordered_map

// node_type extract(key_type const&);

#include <unordered_map>
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
        assert(t.key() == *copy);
        t.key() = *first; // We should be able to mutate key.
        assert(t.key() == *first);
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
        std::unordered_map<int, int> m = {{1,1}, {2,2}, {3,3}, {4,4}, {5,5}, {6,6}};
        int keys[] = {1, 2, 3, 4, 5, 6};
        test(m, std::begin(keys), std::end(keys));
    }

    {
        std::unordered_map<Counter<int>, Counter<int>> m =
            {{1,1}, {2,2}, {3,3}, {4,4}, {5,5}, {6,6}};
        {
            Counter<int> keys[] = {1, 2, 3, 4, 5, 6};
            assert(Counter_base::gConstructed == 12+6);
            test(m, std::begin(keys), std::end(keys));
        }
        assert(Counter_base::gConstructed == 0);
    }

    {
        using min_alloc_map =
            std::unordered_map<int, int, std::hash<int>, std::equal_to<int>,
                               min_allocator<std::pair<const int, int>>>;
        min_alloc_map m = {{1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}, {6, 6}};
        int keys[] = {1, 2, 3, 4, 5, 6};
        test(m, std::begin(keys), std::end(keys));
    }
}
