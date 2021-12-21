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

// class multimap

// iterator insert(node_type&&);

#include <map>
#include <type_traits>
#include "min_allocator.h"

template <class Container>
typename Container::node_type
node_factory(typename Container::key_type const& key,
             typename Container::mapped_type const& mapped)
{
    static Container c;
    auto it = c.insert({key, mapped});
    return c.extract(it);
}

template <class Container>
void test(Container& c)
{
    auto* nf = &node_factory<Container>;

    for (int i = 0; i != 10; ++i)
    {
        typename Container::node_type node = nf(i, i + 1);
        assert(!node.empty());
        typename Container::iterator it = c.insert(std::move(node));
        assert(node.empty());
        assert(it == c.find(i) && it != c.end());
    }

    assert(c.size() == 10);

    { // Insert empty node.
        typename Container::node_type def;
        auto it = c.insert(std::move(def));
        assert(def.empty());
        assert(it == c.end());
    }

    { // Insert duplicate node.
        typename Container::node_type dupl = nf(0, 42);
        auto it = c.insert(std::move(dupl));
        assert(dupl.empty());
        assert(it != c.end());
        assert(it->second == 42);
    }

    assert(c.size() == 11);
    assert(c.count(0) == 2);
    for (int i = 1; i != 10; ++i)
    {
        assert(c.count(i) == 1);
        assert(c.find(i)->second == i + 1);
    }
}

int main()
{
    std::multimap<int, int> m;
    test(m);
    std::multimap<int, int, std::less<int>, min_allocator<std::pair<const int, int>>> m2;
    test(m2);
}
