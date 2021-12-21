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

// template <class Alloc, class U1, class U2>
//   tuple(allocator_arg_t, const Alloc& a, pair<U1, U2>&&);

// UNSUPPORTED: c++98, c++03

#include <tuple>
#include <utility>
#include <memory>
#include <cassert>

#include "allocators.h"
#include "../alloc_first.h"
#include "../alloc_last.h"

struct B
{
    int id_;

    explicit B(int i) : id_(i) {}

    virtual ~B() {}
};

struct D
    : B
{
    explicit D(int i) : B(i) {}
};

int main()
{
    {
        typedef std::pair<int, std::unique_ptr<D>> T0;
        typedef std::tuple<alloc_first, std::unique_ptr<B>> T1;
        T0 t0(2, std::unique_ptr<D>(new D(3)));
        alloc_first::allocator_constructed = false;
        T1 t1(std::allocator_arg, A1<int>(5), std::move(t0));
        assert(alloc_first::allocator_constructed);
        assert(std::get<0>(t1) == 2);
        assert(std::get<1>(t1)->id_ == 3);
    }
}
