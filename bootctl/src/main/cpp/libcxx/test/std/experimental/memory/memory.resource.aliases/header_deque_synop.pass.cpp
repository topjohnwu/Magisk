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

// <experimental/deque>

// namespace std { namespace experimental { namespace pmr {
// template <class T>
// using deque =
//     ::std::deque<T, polymorphic_allocator<T>>
//
// }}} // namespace std::experimental::pmr

#include <experimental/deque>
#include <experimental/memory_resource>
#include <type_traits>
#include <cassert>

namespace pmr = std::experimental::pmr;

int main()
{
    using StdDeque = std::deque<int, pmr::polymorphic_allocator<int>>;
    using PmrDeque = pmr::deque<int>;
    static_assert(std::is_same<StdDeque, PmrDeque>::value, "");
    PmrDeque d;
    assert(d.get_allocator().resource() == pmr::get_default_resource());
}
