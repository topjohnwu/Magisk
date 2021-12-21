// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// REQUIRES: c++experimental
// UNSUPPORTED: c++98, c++03

// <experimental/map>

// namespace std { namespace experimental { namespace pmr {
// template <class K, class V, class Compare = less<Key> >
// using map =
//     ::std::map<K, V, Compare, polymorphic_allocator<pair<const K, V>>>
//
// template <class K, class V, class Compare = less<Key> >
// using multimap =
//     ::std::multimap<K, V, Compare, polymorphic_allocator<pair<const K, V>>>
//
// }}} // namespace std::experimental::pmr

#include <experimental/map>
#include <experimental/memory_resource>
#include <type_traits>
#include <cassert>

namespace pmr = std::experimental::pmr;

int main()
{
    using K = int;
    using V = char;
    using DC = std::less<int>;
    using OC = std::greater<int>;
    using P = std::pair<const K, V>;
    {
        using StdMap = std::map<K, V, DC, pmr::polymorphic_allocator<P>>;
        using PmrMap = pmr::map<K, V>;
        static_assert(std::is_same<StdMap, PmrMap>::value, "");
    }
    {
        using StdMap = std::map<K, V, OC, pmr::polymorphic_allocator<P>>;
        using PmrMap = pmr::map<K, V, OC>;
        static_assert(std::is_same<StdMap, PmrMap>::value, "");
    }
    {
        pmr::map<int, int> m;
        assert(m.get_allocator().resource() == pmr::get_default_resource());
    }
    {
        using StdMap = std::multimap<K, V, DC, pmr::polymorphic_allocator<P>>;
        using PmrMap = pmr::multimap<K, V>;
        static_assert(std::is_same<StdMap, PmrMap>::value, "");
    }
    {
        using StdMap = std::multimap<K, V, OC, pmr::polymorphic_allocator<P>>;
        using PmrMap = pmr::multimap<K, V, OC>;
        static_assert(std::is_same<StdMap, PmrMap>::value, "");
    }
    {
        pmr::multimap<int, int> m;
        assert(m.get_allocator().resource() == pmr::get_default_resource());
    }
}
