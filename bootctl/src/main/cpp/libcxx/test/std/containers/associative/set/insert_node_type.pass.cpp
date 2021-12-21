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

// class set

// insert_return_type insert(node_type&&);

#include <set>
#include <type_traits>
#include "min_allocator.h"

template <class Container>
typename Container::node_type
node_factory(typename Container::key_type const& key)
{
    static Container c;
    auto p = c.insert(key);
    assert(p.second);
    return c.extract(p.first);
}

template <class Container>
void test(Container& c)
{
    auto* nf = &node_factory<Container>;

    for (int i = 0; i != 10; ++i)
    {
        typename Container::node_type node = nf(i);
        assert(!node.empty());
        typename Container::insert_return_type irt = c.insert(std::move(node));
        assert(node.empty());
        assert(irt.inserted);
        assert(irt.node.empty());
        assert(*irt.position == i);
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
        typename Container::node_type dupl = nf(0);
        auto irt = c.insert(std::move(dupl));
        assert(dupl.empty());
        assert(!irt.inserted);
        assert(!irt.node.empty());
        assert(irt.position == c.find(0));
        assert(irt.node.value() == 0);
    }

    assert(c.size() == 10);

    for (int i = 0; i != 10; ++i)
    {
        assert(c.count(i) == 1);
    }
}

int main()
{
    std::set<int> m;
    test(m);
    std::set<int, std::less<int>, min_allocator<int>> m2;
    test(m2);
}
