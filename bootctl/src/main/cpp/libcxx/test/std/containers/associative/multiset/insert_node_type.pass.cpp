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

// iterator insert(node_type&&);

#include <set>
#include <type_traits>
#include "min_allocator.h"

template <class Container>
typename Container::node_type
node_factory(typename Container::key_type const& key)
{
    static Container c;
    auto it = c.insert(key);
    return c.extract(it);
}

template <class Container>
void test(Container& c)
{
    auto* nf = &node_factory<Container>;

    for (int i = 0; i != 10; ++i)
    {
        typename Container::node_type node = nf(i);
        assert(!node.empty());
        typename Container::iterator it = c.insert(std::move(node));
        assert(node.empty());
        assert(it == c.find(i) && it != c.end());
        assert(*it == i);
        assert(node.empty());
    }

    assert(c.size() == 10);

    { // Insert empty node.
        typename Container::node_type def;
        auto it = c.insert(std::move(def));
        assert(def.empty());
        assert(it == c.end());
    }

    { // Insert duplicate node.
        typename Container::node_type dupl = nf(0);
        auto it = c.insert(std::move(dupl));
        assert(*it == 0);
    }

    assert(c.size() == 11);

    assert(c.count(0) == 2);
    for (int i = 1; i != 10; ++i)
    {
        assert(c.count(i) == 1);
    }
}

int main()
{
    std::multiset<int> m;
    test(m);
    std::multiset<int, std::less<int>, min_allocator<int>> m2;
    test(m2);
}
