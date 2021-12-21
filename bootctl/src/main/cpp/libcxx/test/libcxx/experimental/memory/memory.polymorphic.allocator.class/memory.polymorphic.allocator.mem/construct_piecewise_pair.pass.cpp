//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <experimental/memory_resource>

// template <class T> class polymorphic_allocator

// template <class U1, class U2, class ...Args1, class ...Args2>
// void polymorphic_allocator<T>::construct(pair<T1, T2>*, piecewise_construct_t
//                                          tuple<Args1...> x, tuple<Args2...>)

// The stardard specifiers a tranformation to uses-allocator construction as
// follows:
//  - If uses_allocator_v<T1,memory_resource*> is false and
//      is_constructible_v<T,Args1...> is true, then xprime is x.
//  - Otherwise, if uses_allocator_v<T1,memory_resource*> is true and
//      is_constructible_v<T1,allocator_arg_t,memory_resource*,Args1...> is true,
//      then xprime is
//      tuple_cat(make_tuple(allocator_arg, this->resource()), std::move(x)).
//  - Otherwise, if uses_allocator_v<T1,memory_resource*> is true and
//      is_constructible_v<T1,Args1...,memory_resource*> is true, then xprime is
//      tuple_cat(std::move(x), make_tuple(this->resource())).
//  - Otherwise the program is ill formed.
//
// The use of "xprime = tuple_cat(..., std::move(x), ...)" causes all of the
// objects in 'x' to be copied into 'xprime'. If 'x' contains any types which
// are stored by value this causes an unessary copy to occur. To prevent this
//  libc++ changes this call into
// "xprime = forward_as_tuple(..., std::get<Idx>(std::move(x))..., ...)".
// 'xprime' contains references to the values in 'x' instead of copying them.

// This test checks the number of copies incurred to the elements in
// 'tuple<Args1...>' and 'tuple<Args2...>'.

#include <experimental/memory_resource>
#include <type_traits>
#include <utility>
#include <tuple>
#include <cassert>
#include <cstdlib>
#include "test_memory_resource.hpp"

namespace ex = std::experimental::pmr;

template <class T>
struct TestHarness {
    TestResource R;
    ex::memory_resource * M = &R;
    ex::polymorphic_allocator<T> A = M;
    bool constructed = false;
    T * ptr;

    TestHarness() : ptr(A.allocate(1)) {}

    template <class ...Args>
    void construct(Args&&... args) {
        A.construct(ptr, std::forward<Args>(args)...);
        constructed = true;
    }

    ~TestHarness() {
        if (constructed) A.destroy(ptr);
        A.deallocate(ptr, 1);
    }
};

struct CountCopies {
  int count;
  CountCopies() : count(0) {}
  CountCopies(CountCopies const& o) : count(o.count + 1) {}
};

struct CountCopiesAllocV1 {
  typedef ex::polymorphic_allocator<char> allocator_type;
  ex::memory_resource *alloc;
  int count;
  CountCopiesAllocV1() : alloc(nullptr), count(0) {}
  CountCopiesAllocV1(std::allocator_arg_t, allocator_type const& a,
                     CountCopiesAllocV1 const& o) : alloc(a.resource()), count(o.count + 1)
  {}

  CountCopiesAllocV1(CountCopiesAllocV1 const& o) : count(o.count + 1) {}
};


struct CountCopiesAllocV2 {
  typedef ex::polymorphic_allocator<char> allocator_type;
  ex::memory_resource *alloc;
  int count;
  CountCopiesAllocV2() : alloc(nullptr), count(0) {}
  CountCopiesAllocV2(CountCopiesAllocV2 const& o, allocator_type const& a)
    : alloc(a.resource()), count(o.count + 1)
  { }

  CountCopiesAllocV2(CountCopiesAllocV2 const& o) : count(o.count + 1) {}
};


int main()
{
    {
        using T = CountCopies;
        using U = CountCopiesAllocV1;
        using P = std::pair<T, U>;

        std::tuple<T> t1;
        std::tuple<U> t2;

        TestHarness<P> h;
        h.construct(std::piecewise_construct, t1, t2);
        P const& p = *h.ptr;
        assert(p.first.count == 2);
        assert(p.second.count == 2);
        assert(p.second.alloc == h.M);
    }
    {
        using T = CountCopiesAllocV1;
        using U = CountCopiesAllocV2;
        using P = std::pair<T, U>;

        std::tuple<T> t1;
        std::tuple<U> t2;

        TestHarness<P> h;
        h.construct(std::piecewise_construct, std::move(t1), std::move(t2));
        P const& p = *h.ptr;
        assert(p.first.count == 2);
        assert(p.first.alloc == h.M);
        assert(p.second.count == 2);
        assert(p.second.alloc == h.M);
    }
    {
        using T = CountCopiesAllocV2;
        using U = CountCopiesAllocV1;
        using P = std::pair<T, U>;

        std::tuple<T> t1;
        std::tuple<U> t2;

        TestHarness<P> h;
        h.construct(std::piecewise_construct, std::move(t1), std::move(t2));
        P const& p = *h.ptr;
        assert(p.first.count == 2);
        assert(p.first.alloc == h.M);
        assert(p.second.count == 2);
        assert(p.second.alloc == h.M);
    }
    {
        using T = CountCopiesAllocV2;
        using U = CountCopies;
        using P = std::pair<T, U>;

        std::tuple<T> t1;
        std::tuple<U> t2;

        TestHarness<P> h;
        h.construct(std::piecewise_construct, t1, t2);
        P const& p = *h.ptr;
        assert(p.first.count == 2);
        assert(p.first.alloc == h.M);
        assert(p.second.count == 2);
    }
}
