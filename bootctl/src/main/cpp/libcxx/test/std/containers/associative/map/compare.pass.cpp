//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <map>

// template <class Key, class T, class Compare = less<Key>,
//           class Allocator = allocator<pair<const Key, T>>>
// class map

// https://bugs.llvm.org/show_bug.cgi?id=16538
// https://bugs.llvm.org/show_bug.cgi?id=16549

#include <map>
#include <utility>
#include <cassert>

struct Key {
  template <typename T> Key(const T&) {}
  bool operator< (const Key&) const { return false; }
};

int main()
{
    typedef std::map<Key, int> MapT;
    typedef MapT::iterator Iter;
    typedef std::pair<Iter, bool> IterBool;
    {
        MapT m_empty;
        MapT m_contains;
        m_contains[Key(0)] = 42;

        Iter it = m_empty.find(Key(0));
        assert(it == m_empty.end());
        it = m_contains.find(Key(0));
        assert(it != m_contains.end());
    }
    {
        MapT map;
        IterBool result = map.insert(std::make_pair(Key(0), 42));
        assert(result.second);
        assert(result.first->second == 42);
        IterBool result2 = map.insert(std::make_pair(Key(0), 43));
        assert(!result2.second);
        assert(map[Key(0)] == 42);
    }
}
