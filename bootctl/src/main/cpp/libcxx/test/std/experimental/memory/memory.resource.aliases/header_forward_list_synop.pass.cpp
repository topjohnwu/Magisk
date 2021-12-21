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

// <experimental/forward_list>

// namespace std { namespace experimental { namespace pmr {
// template <class T>
// using forward_list =
//     ::std::forward_list<T, polymorphic_allocator<T>>
//
// }}} // namespace std::experimental::pmr

#include <experimental/forward_list>
#include <experimental/memory_resource>
#include <type_traits>
#include <cassert>

namespace pmr = std::experimental::pmr;

int main()
{
    using StdForwardList = std::forward_list<int, pmr::polymorphic_allocator<int>>;
    using PmrForwardList = pmr::forward_list<int>;
    static_assert(std::is_same<StdForwardList, PmrForwardList>::value, "");
    PmrForwardList d;
    assert(d.get_allocator().resource() == pmr::get_default_resource());
}
