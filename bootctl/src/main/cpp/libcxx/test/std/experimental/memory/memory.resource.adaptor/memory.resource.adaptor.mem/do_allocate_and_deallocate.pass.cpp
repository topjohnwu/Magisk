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

// void * do_allocate(size_t size, size_t align)
// void   do_deallocate(void*, size_t, size_t)


#include <experimental/memory_resource>
#include <type_traits>
#include <memory>
#include <exception>
#include <cassert>

#include "test_macros.h"
#include "test_memory_resource.hpp"

namespace ex = std::experimental::pmr;

template <class Alloc>
void check_allocate_deallocate()
{
    typedef ex::resource_adaptor<Alloc> R1;
    const std::size_t max_align = alignof(std::max_align_t);

    for (std::size_t s = 1; s < 5012; ++s)
    {
        for(std::size_t align_req = 1; align_req <= (max_align * 2); align_req *= 2)
        {
            const std::size_t align_exp = align_req > max_align
                                                    ? max_align : align_req;
            AllocController P;
            R1 r{Alloc(P)};
            ex::memory_resource & m1 = r;

            void * const ret = m1.allocate(s, align_req);
            assert(P.alive == 1);
            assert(P.alloc_count == 1);
            assert(P.checkAllocAtLeast(ret, s, align_exp));

            assert(((std::size_t)ret % align_exp) == 0);

            m1.deallocate(ret, s, align_req);
            assert(P.alive == 0);
            assert(P.dealloc_count == 1);
            assert(P.checkDeallocMatchesAlloc());
        }
    }
}

void check_alloc_max_size() {
    using Alloc = NullAllocator<char>;
    using R1 = ex::resource_adaptor<Alloc>;
    const std::size_t max_align = alignof(std::max_align_t);

    auto check = [=](std::size_t s, std::size_t align_req) {
        const std::size_t align_exp = align_req > max_align
                                                ? max_align : align_req;
        AllocController P;
        R1 r{Alloc(P)};
        ex::memory_resource & m1 = r;

        void * const ret = m1.allocate(s, align_req);
        assert(P.alive == 1);
        assert(P.alloc_count == 1);
        assert(P.checkAllocAtLeast(ret, s, align_exp));

        m1.deallocate(ret, s, align_req);
        assert(P.alive == 0);
        assert(P.dealloc_count == 1);
        assert(P.checkDeallocMatchesAlloc());
    };

    const std::size_t sizeTypeMax = ~0;
    const std::size_t testSizeStart = sizeTypeMax - (max_align * 3);
    const std::size_t testSizeEnd = sizeTypeMax - max_align;

    for (std::size_t size = testSizeStart; size <= testSizeEnd; ++size) {
        for (std::size_t align=1; align <= (max_align * 2); align *= 2) {
            check(size, align);
        }
    }

#ifndef TEST_HAS_NO_EXCEPTIONS
    for (std::size_t size = sizeTypeMax; size > testSizeEnd; --size) {
        AllocController P;
        R1 r{Alloc(P)};
        ex::memory_resource & m1 = r;

        try {
            m1.allocate(size);
            assert(false);
        } catch (std::exception const&) {
        }
    }
#endif
}

int main()
{
    check_allocate_deallocate<CountingAllocator<char>>();
    check_allocate_deallocate<MinAlignedAllocator<char>>();
    check_alloc_max_size();
}
