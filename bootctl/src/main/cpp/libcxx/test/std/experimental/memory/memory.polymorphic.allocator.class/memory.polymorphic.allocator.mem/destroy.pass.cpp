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

// template <class U>
// void polymorphic_allocator<T>::destroy(U * ptr);

#include <experimental/memory_resource>
#include <type_traits>
#include <new>
#include <cassert>
#include <cstdlib>

namespace ex = std::experimental::pmr;

int count = 0;

struct destroyable
{
    destroyable() { ++count; }
    ~destroyable() { --count; }
};

int main()
{
    typedef ex::polymorphic_allocator<double> A;
    {
        A a;
        static_assert(
            std::is_same<decltype(a.destroy((destroyable*)nullptr)), void>::value,
            "");
    }
    {
        destroyable * ptr = ::new (std::malloc(sizeof(destroyable))) destroyable();
        assert(count == 1);
        A{}.destroy(ptr);
        assert(count == 0);
        std::free(ptr);
    }
}
