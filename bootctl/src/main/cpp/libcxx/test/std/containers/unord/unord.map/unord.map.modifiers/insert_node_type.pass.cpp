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

// insert_return_type insert(node_type&&);

#include <unordered_map>
#include "min_allocator.h"

template <class Container>
typename Container::node_type
node_factory(typename Container::key_type const& key,
             typename Container::mapped_type const& mapped)
{
    static Container c;
    auto p = c.insert({key, mapped});
    assert(p.second);
    return c.extract(p.first);
}

template <class Container>
void test(Container& c)
{
    auto* nf = &node_factory<Container>;

    for (int i = 0; i != 10; ++i)
    {
        typename Container::node_type node = nf(i, i + 1);
        assert(!node.empty());
        typename Container::insert_return_type irt = c.insert(std::move(node));
        assert(node.empty());
        assert(irt.inserted);
        assert(irt.node.empty());
        assert(irt.position->first == i && irt.position->second == i + 1);
    }

    assert(c.size() == 10);

    { // Insert empty node.
        typename Container::node_type def;
        auto irt = c.insert(std::move(def));
        assert(def.empty());
        assert(!irt.inserted);
        assert(irt.node.empty());
        assert(irt.position == c.end());
    }

    { // Insert duplicate node.
        typename Container::node_type dupl = nf(0, 42);
        auto irt = c.insert(std::move(dupl));
        assert(dupl.empty());
        assert(!irt.inserted);
        assert(!irt.node.empty());
        assert(irt.position == c.find(0));
        assert(irt.node.key() == 0 && irt.node.mapped() == 42);
    }

    assert(c.size() == 10);

    for (int i = 0; i != 10; ++i)
    {
        assert(c.count(i) == 1);
        assert(c[i] == i + 1);
    }
}

int main()
{
    std::unordered_map<int, int> m;
    test(m);
    std::unordered_map<int, int, std::hash<int>, std::equal_to<int>, min_allocator<std::pair<const int, int>>> m2;
    test(m2);
}
