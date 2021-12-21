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
// void polymorphic_allocator<T>::construct(pair<P1, P2>*, U1&&, U2&&)

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
            TT&& t, UU&& u)
{
    using P = std::pair<UA1, UA2>;
    TestResource R;
    ex::memory_resource * M = &R;
    ex::polymorphic_allocator<P> A(M);
    P * ptr = (P*)std::malloc(sizeof(P));
    P * ptr2 = (P*)std::malloc(sizeof(P));

    // UNDER TEST //
    A.construct(ptr, std::forward<TT>(t), std::forward<UU>(u));
    A.construct(ptr2, std::piecewise_construct,
                      std::forward_as_tuple(std::forward<TT>(t)),
                      std::forward_as_tuple(std::forward<UU>(u)));
    // ------- //

    bool tres = checkConstruct<TT&&>(ptr->first, TExpect, M) &&
                checkConstructionEquiv(ptr->first, ptr2->first);

    bool ures = checkConstruct<UU&&>(ptr->second, UExpect, M) &&
                checkConstructionEquiv(ptr->second, ptr2->second);

    A.destroy(ptr);
    A.destroy(ptr2);
    std::free(ptr);
    std::free(ptr2);
    return tres && ures;
}

template <class Alloc, class TT, class UU>
void test_pmr_uses_allocator(TT&& t, UU&& u)
{
    {
        using T = NotUsesAllocator<Alloc, 1>;
        using U = NotUsesAllocator<Alloc, 1>;
        assert((doTest<T, U>(UA_None, UA_None,
                             std::forward<TT>(t), std::forward<UU>(u))));
    }
    {
        using T = UsesAllocatorV1<Alloc, 1>;
        using U = UsesAllocatorV2<Alloc, 1>;
        assert((doTest<T, U>(UA_AllocArg, UA_AllocLast,
                             std::forward<TT>(t), std::forward<UU>(u))));
    }
    {
        using T = UsesAllocatorV2<Alloc, 1>;
        using U = UsesAllocatorV3<Alloc, 1>;
        assert((doTest<T, U>(UA_AllocLast, UA_AllocArg,
                             std::forward<TT>(t), std::forward<UU>(u))));
    }
    {
        using T = UsesAllocatorV3<Alloc, 1>;
        using U = NotUsesAllocator<Alloc, 1>;
        assert((doTest<T, U>(UA_AllocArg, UA_None,
                             std::forward<TT>(t), std::forward<UU>(u))));
    }
}

template <class Alloc, class TT, class UU>
void test_pmr_not_uses_allocator(TT&& t, UU&& u)
{
    {
        using T = NotUsesAllocator<Alloc, 1>;
        using U = NotUsesAllocator<Alloc, 1>;
        assert((doTest<T, U>(UA_None, UA_None,
                             std::forward<TT>(t), std::forward<UU>(u))));
    }
    {
        using T = UsesAllocatorV1<Alloc, 1>;
        using U = UsesAllocatorV2<Alloc, 1>;
        assert((doTest<T, U>(UA_None, UA_None,
                             std::forward<TT>(t), std::forward<UU>(u))));
    }
    {
        using T = UsesAllocatorV2<Alloc, 1>;
        using U = UsesAllocatorV3<Alloc, 1>;
        assert((doTest<T, U>(UA_None, UA_None,
                             std::forward<TT>(t), std::forward<UU>(u))));
    }
    {
        using T = UsesAllocatorV3<Alloc, 1>;
        using U = NotUsesAllocator<Alloc, 1>;
        assert((doTest<T, U>(UA_None, UA_None,
                             std::forward<TT>(t), std::forward<UU>(u))));
    }
}

int main()
{
    using ERT = std::experimental::erased_type;
    using PMR = ex::memory_resource*;
    using PMA = ex::polymorphic_allocator<char>;
    {
        int x = 42;
        int y = 42;
        test_pmr_uses_allocator<ERT>(x, std::move(y));
        test_pmr_not_uses_allocator<PMR>(x, std::move(y));
        test_pmr_uses_allocator<PMA>(x, std::move(y));
    }
    {
        int x = 42;
        const int y = 42;
        test_pmr_uses_allocator<ERT>(std::move(x), y);
        test_pmr_not_uses_allocator<PMR>(std::move(x), y);
        test_pmr_uses_allocator<PMA>(std::move(x), y);
    }
}
