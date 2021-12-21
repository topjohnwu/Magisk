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
//   tuple(allocator_arg_t, const Alloc& a, const tuple&);

// UNSUPPORTED: c++98, c++03

#include <tuple>
#include <cassert>

#include "allocators.h"
#include "../alloc_first.h"
#include "../alloc_last.h"

int main()
{
    {
        typedef std::tuple<> T;
        T t0;
        T t(std::allocator_arg, A1<int>(), t0);
    }
    {
        typedef std::tuple<int> T;
        T t0(2);
        T t(std::allocator_arg, A1<int>(), t0);
        assert(std::get<0>(t) == 2);
    }
    {
        typedef std::tuple<alloc_first> T;
        T t0(2);
        alloc_first::allocator_constructed = false;
        T t(std::allocator_arg, A1<int>(5), t0);
        assert(alloc_first::allocator_constructed);
        assert(std::get<0>(t) == 2);
    }
    {
        typedef std::tuple<alloc_last> T;
        T t0(2);
        alloc_last::allocator_constructed = false;
        T t(std::allocator_arg, A1<int>(5), t0);
        assert(alloc_last::allocator_constructed);
        assert(std::get<0>(t) == 2);
    }
// testing extensions
#ifdef _LIBCPP_VERSION
    {
        typedef std::tuple<alloc_first, alloc_last> T;
        T t0(2, 3);
        alloc_first::allocator_constructed = false;
        alloc_last::allocator_constructed = false;
        T t(std::allocator_arg, A1<int>(5), t0);
        assert(alloc_first::allocator_constructed);
        assert(alloc_last::allocator_constructed);
        assert(std::get<0>(t) == 2);
        assert(std::get<1>(t) == 3);
    }
    {
        typedef std::tuple<int, alloc_first, alloc_last> T;
        T t0(1, 2, 3);
        alloc_first::allocator_constructed = false;
        alloc_last::allocator_constructed = false;
        T t(std::allocator_arg, A1<int>(5), t0);
        assert(alloc_first::allocator_constructed);
        assert(alloc_last::allocator_constructed);
        assert(std::get<0>(t) == 1);
        assert(std::get<1>(t) == 2);
        assert(std::get<2>(t) == 3);
    }
#endif
}
