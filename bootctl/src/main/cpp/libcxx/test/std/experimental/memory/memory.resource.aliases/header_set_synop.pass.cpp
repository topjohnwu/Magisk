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

// <experimental/set>

// namespace std { namespace experimental { namespace pmr {
// template <class V, class Compare = less<V> >
// using set =
//     ::std::set<V, Compare, polymorphic_allocator<V>>
//
// template <class V, class Compare = less<V> >
// using multiset =
//     ::std::multiset<V, Compare, polymorphic_allocator<V>>
//
// }}} // namespace std::experimental::pmr

#include <experimental/set>
#include <experimental/memory_resource>
#include <type_traits>
#include <cassert>

namespace pmr = std::experimental::pmr;

int main()
{
    using V = char;
    using DC = std::less<V>;
    using OC = std::greater<V>;
    {
        using StdSet = std::set<V, DC, pmr::polymorphic_allocator<V>>;
        using PmrSet = pmr::set<V>;
        static_assert(std::is_same<StdSet, PmrSet>::value, "");
    }
    {
        using StdSet = std::set<V, OC, pmr::polymorphic_allocator<V>>;
        using PmrSet = pmr::set<V, OC>;
        static_assert(std::is_same<StdSet, PmrSet>::value, "");
    }
    {
        pmr::set<int> m;
        assert(m.get_allocator().resource() == pmr::get_default_resource());
    }
    {
        using StdSet = std::multiset<V, DC, pmr::polymorphic_allocator<V>>;
        using PmrSet = pmr::multiset<V>;
        static_assert(std::is_same<StdSet, PmrSet>::value, "");
    }
    {
        using StdSet = std::multiset<V, OC, pmr::polymorphic_allocator<V>>;
        using PmrSet = pmr::multiset<V, OC>;
        static_assert(std::is_same<StdSet, PmrSet>::value, "");
    }
    {
        pmr::multiset<int> m;
        assert(m.get_allocator().resource() == pmr::get_default_resource());
    }
}
