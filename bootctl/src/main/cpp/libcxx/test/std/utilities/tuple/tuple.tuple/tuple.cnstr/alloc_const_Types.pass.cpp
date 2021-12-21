//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <tuple>

// template <class... Types> class tuple;

// template <class Alloc>
//   tuple(allocator_arg_t, const Alloc& a, const Types&...);

// UNSUPPORTED: c++98, c++03

#include <tuple>
#include <memory>
#include <cassert>

#include "allocators.h"
#include "../alloc_first.h"
#include "../alloc_last.h"

struct ImplicitCopy {
  explicit ImplicitCopy(int) {}
  ImplicitCopy(ImplicitCopy const&) {}
};

// Test that tuple(std::allocator_arg, Alloc, Types const&...) allows implicit
// copy conversions in return value expressions.
std::tuple<ImplicitCopy> testImplicitCopy1() {
    ImplicitCopy i(42);
    return {std::allocator_arg, std::allocator<void>{}, i};
}

std::tuple<ImplicitCopy> testImplicitCopy2() {
    const ImplicitCopy i(42);
    return {std::allocator_arg, std::allocator<void>{}, i};
}

int main()
{
    {
        // check that the literal '0' can implicitly initialize a stored pointer.
        std::tuple<int*> t = {std::allocator_arg, std::allocator<void>{}, 0};
    }
    {
        std::tuple<int> t(std::allocator_arg, A1<int>(), 3);
        assert(std::get<0>(t) == 3);
    }
    {
        assert(!alloc_first::allocator_constructed);
        std::tuple<alloc_first> t(std::allocator_arg, A1<int>(5), alloc_first(3));
        assert(alloc_first::allocator_constructed);
        assert(std::get<0>(t) == alloc_first(3));
    }
    {
        assert(!alloc_last::allocator_constructed);
        std::tuple<alloc_last> t(std::allocator_arg, A1<int>(5), alloc_last(3));
        assert(alloc_last::allocator_constructed);
        assert(std::get<0>(t) == alloc_last(3));
    }
    {
        alloc_first::allocator_constructed = false;
        std::tuple<int, alloc_first> t(std::allocator_arg, A1<int>(5),
                                       10, alloc_first(15));
        assert(std::get<0>(t) == 10);
        assert(alloc_first::allocator_constructed);
        assert(std::get<1>(t) == alloc_first(15));
    }
    {
        alloc_first::allocator_constructed = false;
        alloc_last::allocator_constructed = false;
        std::tuple<int, alloc_first, alloc_last> t(std::allocator_arg,
                                                   A1<int>(5), 1, alloc_first(2),
                                                   alloc_last(3));
        assert(std::get<0>(t) == 1);
        assert(alloc_first::allocator_constructed);
        assert(std::get<1>(t) == alloc_first(2));
        assert(alloc_last::allocator_constructed);
        assert(std::get<2>(t) == alloc_last(3));
    }
    {
        alloc_first::allocator_constructed = false;
        alloc_last::allocator_constructed = false;
        std::tuple<int, alloc_first, alloc_last> t(std::allocator_arg,
                                                   A2<int>(5), 1, alloc_first(2),
                                                   alloc_last(3));
        assert(std::get<0>(t) == 1);
        assert(!alloc_first::allocator_constructed);
        assert(std::get<1>(t) == alloc_first(2));
        assert(!alloc_last::allocator_constructed);
        assert(std::get<2>(t) == alloc_last(3));
    }
}
