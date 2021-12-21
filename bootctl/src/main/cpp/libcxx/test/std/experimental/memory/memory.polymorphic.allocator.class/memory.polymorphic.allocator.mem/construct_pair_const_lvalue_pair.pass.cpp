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

// template <class P1, class P2, class U1, class U2>
// void polymorphic_allocator<T>::construct(pair<P1, P2>*, pair<U1, U2> const&)

#include <experimental/memory_resource>
#include <type_traits>
#include <utility>
#include <tuple>
#include <cassert>
#include <cstdlib>

#include "test_macros.h"
#include "test_memory_resource.hpp"
#include "uses_alloc_types.hpp"
#include "controlled_allocators.hpp"
#include "test_allocator.h"

namespace ex = std::experimental::pmr;


template <class UA1, class UA2, class TT, class UU>
bool doTest(UsesAllocatorType TExpect, UsesAllocatorType UExpect,
            std::pair<TT, UU> const& p)
{
    using P = std::pair<UA1, UA2>;
    TestResource R;
    ex::memory_resource * M = &R;
    ex::polymorphic_allocator<P> A(M);
    P * ptr = (P*)std::malloc(sizeof(P));
    P * ptr2 =  (P*)std::malloc(sizeof(P));

    // UNDER TEST //
    A.construct(ptr, p);

    A.construct(ptr2, std::piecewise_construct,
                std::forward_as_tuple(p.first),
                std::forward_as_tuple(p.second));
    // ------- //

    bool tres = checkConstruct<decltype((p.first))>(ptr->first, TExpect, M) &&
                checkConstructionEquiv(ptr->first, ptr2->first);

    bool ures = checkConstruct<decltype((p.second))>(ptr->second, UExpect, M) &&
                checkConstructionEquiv(ptr->second, ptr2->second);

    A.destroy(ptr);
    std::free(ptr);
    A.destroy(ptr2);
    std::free(ptr2);
    return tres && ures;

}

template <class Alloc, class TT, class UU>
void test_pmr_uses_allocator(std::pair<TT, UU> const& p)
{
    {
        using T = NotUsesAllocator<Alloc, 1>;
        using U = NotUsesAllocator<Alloc, 1>;
        assert((doTest<T, U>(UA_None, UA_None, p)));
    }
    {
        using T = UsesAllocatorV1<Alloc, 1>;
        using U = UsesAllocatorV2<Alloc, 1>;
        assert((doTest<T, U>(UA_AllocArg, UA_AllocLast, p)));
    }
    {
        using T = UsesAllocatorV2<Alloc, 1>;
        using U = UsesAllocatorV3<Alloc, 1>;
        assert((doTest<T, U>(UA_AllocLast, UA_AllocArg, p)));
    }
    {
        using T = UsesAllocatorV3<Alloc, 1>;
        using U = NotUsesAllocator<Alloc, 1>;
        assert((doTest<T, U>(UA_AllocArg, UA_None, p)));
    }
}

template <class Alloc, class TT, class UU>
void test_pmr_not_uses_allocator(std::pair<TT, UU> const& p)
{
    {
        using T = NotUsesAllocator<Alloc, 1>;
        using U = NotUsesAllocator<Alloc, 1>;
        assert((doTest<T, U>(UA_None, UA_None, p)));
    }
    {
        using T = UsesAllocatorV1<Alloc, 1>;
        using U = UsesAllocatorV2<Alloc, 1>;
        assert((doTest<T, U>(UA_None, UA_None, p)));
    }
    {
        using T = UsesAllocatorV2<Alloc, 1>;
        using U = UsesAllocatorV3<Alloc, 1>;
        assert((doTest<T, U>(UA_None, UA_None, p)));
    }
    {
        using T = UsesAllocatorV3<Alloc, 1>;
        using U = NotUsesAllocator<Alloc, 1>;
        assert((doTest<T, U>(UA_None, UA_None, p)));
    }
}

template <class Tp>
struct Print;

int main()
{
    using ERT = std::experimental::erased_type;
    using PMR = ex::memory_resource*;
    using PMA = ex::polymorphic_allocator<char>;
    {
        int x = 42;
        int y = 42;
        const std::pair<int, int&> p(x, y);
        test_pmr_uses_allocator<ERT>(p);
        test_pmr_not_uses_allocator<PMR>(p);
        test_pmr_uses_allocator<PMA>(p);
    }
    {
        int x = 42;
        int y = 42;
        const std::pair<int&, int&&> p(x, std::move(y));
        test_pmr_uses_allocator<ERT>(p);
        test_pmr_not_uses_allocator<PMR>(p);
        test_pmr_uses_allocator<PMA>(p);
    }
}
