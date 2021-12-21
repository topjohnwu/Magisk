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

// template <class Alloc> class resource_adaptor_imp;

// bool do_is_equal(memory_resource const &) const noexcept;

#include <experimental/memory_resource>
#include <type_traits>
#include <memory>
#include <cassert>
#include "test_memory_resource.hpp"

using std::size_t;
namespace ex = std::experimental::pmr;

int main()
{

    typedef CountingAllocator<char> Alloc1;
    typedef CountingAllocator<int> RAlloc1;
    typedef ex::resource_adaptor<Alloc1> R1;
    typedef ex::resource_adaptor<RAlloc1> RR1;
    static_assert(std::is_same<R1, RR1>::value, "");

    typedef std::allocator<char> Alloc2;
    typedef ex::resource_adaptor<Alloc2> R2;
    static_assert(!std::is_same<R1, R2>::value, "");

    // equal same type
    {
        AllocController C;
        Alloc1 a1(C);
        R1 const r1(a1);
        ex::memory_resource const & m1 = r1;

        Alloc1 a2(C);
        R1 const r2(a2);
        ex::memory_resource const & m2 = r2;

        assert(m1.is_equal(m2));
        assert(m2.is_equal(m1));
    }
    // not equal same type
    {
        AllocController C;
        Alloc1 a1(C);
        R1 const r1(a1);
        ex::memory_resource const & m1 = r1;

        AllocController C2;
        Alloc1 a2(C2);
        R1 const r2(a2);
        ex::memory_resource const & m2 = r2;

        assert(!m1.is_equal(m2));
        assert(!m2.is_equal(m1));
    }
    // different allocator types
    {
        AllocController C;
        Alloc1 a1(C);
        R1 const r1(a1);
        ex::memory_resource const & m1 = r1;

        Alloc2 a2;
        R2 const r2(a2);
        ex::memory_resource const & m2 = r2;

        assert(!m1.is_equal(m2));
        assert(!m2.is_equal(m1));
    }
}
