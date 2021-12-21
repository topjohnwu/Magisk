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

// iterator insert(const_iterator hint, node_type&&);

#include <set>
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
        size_t prev = c.size();
        auto it = c.insert(c.end(), std::move(node));
        assert(prev + 1 == c.size());
        assert(*it == i);
    }

    assert(c.size() == 10);

    for (int i = 0; i != 10; ++i)
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
