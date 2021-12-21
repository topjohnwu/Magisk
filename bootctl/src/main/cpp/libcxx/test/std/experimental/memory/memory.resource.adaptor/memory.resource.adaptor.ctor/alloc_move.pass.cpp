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

// resource_adaptor_imp<Alloc>::resource_adaptor_imp(Alloc &&)

#include <experimental/memory_resource>
#include <cassert>

#include "test_memory_resource.hpp"

namespace ex = std::experimental::pmr;

int main()
{
    typedef CountingAllocator<char> AllocT;
    typedef ex::resource_adaptor<AllocT> R;
    {
        AllocController P;
        AllocT a(P);
        R const r(std::move(a));
        assert(P.copy_constructed == 0);
        assert(P.move_constructed == 1);
        assert(r.get_allocator() == a);
    }
    {
        AllocController P;
        R const r(AllocT{P});
        assert(P.copy_constructed == 0);
        assert(P.move_constructed == 1);
        assert(r.get_allocator() == AllocT{P});
    }
}
