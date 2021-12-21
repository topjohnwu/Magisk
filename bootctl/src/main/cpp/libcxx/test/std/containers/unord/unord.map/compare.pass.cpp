//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <unordered_map>

// template <class Key, class T, class Hash = hash<Key>, class Pred = equal_to<Key>,
//           class Alloc = allocator<pair<const Key, T>>>
// class unordered_map

// https://bugs.llvm.org/show_bug.cgi?id=16538
// https://bugs.llvm.org/show_bug.cgi?id=16549

#include <unordered_map>
#include <cassert>

struct Key {
  template <typename T> Key(const T&) {}
  bool operator== (const Key&) const { return true; }
};

namespace std
{
    template <>
    struct hash<Key>
    {
        size_t operator()(Key const &) const {return 0;}
    };
}

int main()
{
    typedef std::unordered_map<Key, int> MapT;
    typedef MapT::iterator Iter;
    MapT map;
    Iter it = map.find(Key(0));
    assert(it == map.end());
    std::pair<Iter, bool> result = map.insert(std::make_pair(Key(0), 42));
    assert(result.second);
    assert(result.first->second == 42);
}
