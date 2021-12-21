//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14

// <map>

// class map

// node_type extract(const_iterator);

#include <map>
#include "min_allocator.h"
#include "Counter.h"

template <class Container>
void test(Container& c)
{
    size_t sz = c.size();

    auto some_key = c.cbegin()->first;

    for (auto first = c.cbegin(); first != c.cend();)
    {
        auto key_value = first->first;
        typename Container::node_type t = c.extract(first++);
        --sz;
        assert(t.key() == key_value);
        t.key() = some_key;
        assert(t.key() == some_key);
        assert(t.get_allocator() == c.get_allocator());
        assert(sz == c.size());
    }

    assert(c.size() == 0);
}

int main()
{
    {
        using map_type = std::map<int, int>;
        map_type m = {{1,1}, {2,2}, {3,3}, {4,4}, {5,5}, {6,6}};
        test(m);
    }

    {
        std::map<Counter<int>, Counter<int>> m =
            {{1,1}, {2,2}, {3,3}, {4,4}, {5,5}, {6,6}};
        assert(Counter_base::gConstructed == 12);
        test(m);
        assert(Counter_base::gConstructed == 0);
    }

    {
        using min_alloc_map =
            std::map<int, int, std::less<int>,
                     min_allocator<std::pair<const int, int>>>;
        min_alloc_map m = {{1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}, {6, 6}};
        test(m);
    }
}
