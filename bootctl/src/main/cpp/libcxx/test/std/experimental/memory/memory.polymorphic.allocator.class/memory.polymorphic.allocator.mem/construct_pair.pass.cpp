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

// <experimental/memory_resource>

// template <class T> class polymorphic_allocator

// template <class U1, class U2>
// void polymorphic_allocator<T>::construct(pair<U1, U2>*)

#include <experimental/memory_resource>
#include <type_traits>
#include <utility>
#include <tuple>
#include <cassert>
#include <cstdlib>
#include "uses_alloc_types.hpp"

namespace ex = std::experimental::pmr;

int constructed = 0;

struct default_constructible
{
    default_constructible() : x(42)  { ++constructed; }
    int x{0};
};

int main()
{
    // pair<default_constructible, default_constructible> as T()
    {
        typedef default_constructible T;
        typedef std::pair<T, T> P;
        typedef ex::polymorphic_allocator<void> A;
        P * ptr = (P*)std::malloc(sizeof(P));
        A a;
        a.construct(ptr);
        assert(constructed == 2);
        assert(ptr->first.x == 42);
        assert(ptr->second.x == 42);
        std::free(ptr);
    }
}
