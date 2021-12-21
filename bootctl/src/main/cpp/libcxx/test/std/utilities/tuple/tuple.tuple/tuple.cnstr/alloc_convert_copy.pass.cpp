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

// template <class Alloc, class... UTypes>
//   tuple(allocator_arg_t, const Alloc& a, const tuple<UTypes...>&);

// UNSUPPORTED: c++98, c++03

#include <tuple>
#include <memory>
#include <cassert>

#include "allocators.h"
#include "../alloc_first.h"
#include "../alloc_last.h"

struct Explicit {
  int value;
  explicit Explicit(int x) : value(x) {}
};

struct Implicit {
  int value;
  Implicit(int x) : value(x) {}
};

int main()
{
    {
        typedef std::tuple<long> T0;
        typedef std::tuple<long long> T1;
        T0 t0(2);
        T1 t1(std::allocator_arg, A1<int>(), t0);
        assert(std::get<0>(t1) == 2);
    }
    {
        typedef std::tuple<int> T0;
        typedef std::tuple<alloc_first> T1;
        T0 t0(2);
        alloc_first::allocator_constructed = false;
        T1 t1(std::allocator_arg, A1<int>(5), t0);
        assert(alloc_first::allocator_constructed);
        assert(std::get<0>(t1) == 2);
    }
    {
        typedef std::tuple<int, int> T0;
        typedef std::tuple<alloc_first, alloc_last> T1;
        T0 t0(2, 3);
        alloc_first::allocator_constructed = false;
        alloc_last::allocator_constructed = false;
        T1 t1(std::allocator_arg, A1<int>(5), t0);
        assert(alloc_first::allocator_constructed);
        assert(alloc_last::allocator_constructed);
        assert(std::get<0>(t1) == 2);
        assert(std::get<1>(t1) == 3);
    }
    {
        typedef std::tuple<long, int, int> T0;
        typedef std::tuple<long long, alloc_first, alloc_last> T1;
        T0 t0(1, 2, 3);
        alloc_first::allocator_constructed = false;
        alloc_last::allocator_constructed = false;
        T1 t1(std::allocator_arg, A1<int>(5), t0);
        assert(alloc_first::allocator_constructed);
        assert(alloc_last::allocator_constructed);
        assert(std::get<0>(t1) == 1);
        assert(std::get<1>(t1) == 2);
        assert(std::get<2>(t1) == 3);
    }
    {
        const std::tuple<int> t1(42);
        std::tuple<Explicit> t2{std::allocator_arg, std::allocator<void>{},  t1};
        assert(std::get<0>(t2).value == 42);
    }
    {
        const std::tuple<int> t1(42);
        std::tuple<Implicit> t2 = {std::allocator_arg, std::allocator<void>{}, t1};
        assert(std::get<0>(t2).value == 42);
    }
}
