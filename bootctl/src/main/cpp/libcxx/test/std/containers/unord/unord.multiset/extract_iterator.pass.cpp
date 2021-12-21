//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14

// <unordered_set>

// class unordered_multiset

// node_type extract(const_iterator);

#include <unordered_set>
#include "min_allocator.h"
#include "Counter.h"

template <class Container>
void test(Container& c)
{
    size_t sz = c.size();

    for (auto first = c.cbegin(); first != c.cend();)
    {
        auto key_value = *first;
        typename Container::node_type t = c.extract(first++);
        --sz;
        assert(t.value() == key_value);
        assert(t.get_allocator() == c.get_allocator());
        assert(sz == c.size());
    }

    assert(c.size() == 0);
}

int main()
{
    {
        using set_type = std::unordered_multiset<int>;
        set_type m = {1, 2, 3, 4, 5, 6};
        test(m);
    }

    {
        std::unordered_multiset<Counter<int>> m = {1, 2, 3, 4, 5, 6};
        assert(Counter_base::gConstructed == 6);
        test(m);
        assert(Counter_base::gConstructed == 0);
    }

    {
        using min_alloc_set = std::unordered_multiset<int, std::hash<int>, std::equal_to<int>, min_allocator<int>>;
        min_alloc_set m = {1, 2, 3, 4, 5, 6};
        test(m);
    }
}
