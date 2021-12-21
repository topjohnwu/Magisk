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

// <experimental/unordered_map>

// namespace std { namespace experimental { namespace pmr {
// template <class K, class V, class H = hash<K>, class P = equal_to<K> >
// using unordered_map =
//     ::std::unordered_map<K, V, H, P, polymorphic_allocator<pair<const K, V>>>
//
// template <class K, class V,  class H = hash<K>, class P = equal_to<K> >
// using unordered_multimap =
//     ::std::unordered_multimap<K, V, H, P, polymorphic_allocator<pair<const K, V>>>
//
// }}} // namespace std::experimental::pmr

#include <experimental/unordered_map>
#include <experimental/memory_resource>
#include <type_traits>
#include <cassert>

namespace pmr = std::experimental::pmr;

template <class T>
struct MyHash : std::hash<T> {};

template <class T>
struct MyPred : std::equal_to<T> {};

int main()
{
    using K = int;
    using V = char;
    using DH = std::hash<K>;
    using MH = MyHash<K>;
    using DP = std::equal_to<K>;
    using MP = MyPred<K>;
    using P = std::pair<const K, V>;
    {
        using StdMap = std::unordered_map<K, V, DH, DP, pmr::polymorphic_allocator<P>>;
        using PmrMap = pmr::unordered_map<K, V>;
        static_assert(std::is_same<StdMap, PmrMap>::value, "");
    }
    {
        using StdMap = std::unordered_map<K, V, MH, DP, pmr::polymorphic_allocator<P>>;
        using PmrMap = pmr::unordered_map<K, V, MH>;
        static_assert(std::is_same<StdMap, PmrMap>::value, "");
    }
    {
        using StdMap = std::unordered_map<K, V, MH, MP, pmr::polymorphic_allocator<P>>;
        using PmrMap = pmr::unordered_map<K, V, MH, MP>;
        static_assert(std::is_same<StdMap, PmrMap>::value, "");
    }
    {
        pmr::unordered_map<int, int> m;
        assert(m.get_allocator().resource() == pmr::get_default_resource());
    }
    {
        using StdMap = std::unordered_multimap<K, V, DH, DP, pmr::polymorphic_allocator<P>>;
        using PmrMap = pmr::unordered_multimap<K, V>;
        static_assert(std::is_same<StdMap, PmrMap>::value, "");
    }
    {
        using StdMap = std::unordered_multimap<K, V, MH, DP, pmr::polymorphic_allocator<P>>;
        using PmrMap = pmr::unordered_multimap<K, V, MH>;
        static_assert(std::is_same<StdMap, PmrMap>::value, "");
    }
    {
        using StdMap = std::unordered_multimap<K, V, MH, MP, pmr::polymorphic_allocator<P>>;
        using PmrMap = pmr::unordered_multimap<K, V, MH, MP>;
        static_assert(std::is_same<StdMap, PmrMap>::value, "");
    }
    {
        pmr::unordered_multimap<int, int> m;
        assert(m.get_allocator().resource() == pmr::get_default_resource());
    }
}
