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

// <experimental/vector>

// namespace std { namespace experimental { namespace pmr {
// template <class T>
// using vector =
//     ::std::vector<T, polymorphic_allocator<T>>
//
// }}} // namespace std::experimental::pmr

#include <experimental/vector>
#include <experimental/memory_resource>
#include <type_traits>
#include <cassert>

namespace pmr = std::experimental::pmr;

int main()
{
    using StdVector = std::vector<int, pmr::polymorphic_allocator<int>>;
    using PmrVector = pmr::vector<int>;
    static_assert(std::is_same<StdVector, PmrVector>::value, "");
    PmrVector d;
    assert(d.get_allocator().resource() == pmr::get_default_resource());
}
