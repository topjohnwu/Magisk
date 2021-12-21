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

// template <class U, class ...Args>
// void polymorphic_allocator<T>::construct(U *, Args &&...)

#include <experimental/memory_resource>
#include <type_traits>
#include <cassert>
#include <cstdlib>

#include "test_macros.h"
#include "test_memory_resource.hpp"
#include "uses_alloc_types.hpp"
#include "controlled_allocators.hpp"
#include "test_allocator.h"

namespace ex = std::experimental::pmr;

template <class T>
struct PMATest {
    TestResource R;
    ex::polymorphic_allocator<T> A;
    T* ptr;
    bool constructed;

    PMATest() : A(&R), ptr(A.allocate(1)), constructed(false) {}

    template <class ...Args>
    void construct(Args&&... args) {
        A.construct(ptr, std::forward<Args>(args)...);
        constructed = true;
    }

    ~PMATest() {
        if (constructed) A.destroy(ptr);
        A.deallocate(ptr, 1);
    }
};

template <class T, class ...Args>
bool doTest(UsesAllocatorType UAExpect, Args&&... args)
{
    PMATest<T> TH;
    // UNDER TEST //
    TH.construct(std::forward<Args>(args)...);
    return checkConstruct<Args&&...>(*TH.ptr, UAExpect, &TH.R);
    // ------- //
}


template <class T, class ...Args>
bool doTestUsesAllocV0(Args&&... args)
{
    PMATest<T> TH;
    // UNDER TEST //
    TH.construct(std::forward<Args>(args)...);
    return checkConstruct<Args&&...>(*TH.ptr, UA_None);
    // -------- //
}


template <class T, class EAlloc, class ...Args>
bool doTestUsesAllocV1(EAlloc const& ealloc, Args&&... args)
{
    PMATest<T> TH;
    // UNDER TEST //
    TH.construct(std::allocator_arg, ealloc, std::forward<Args>(args)...);
    return checkConstruct<Args&&...>(*TH.ptr, UA_AllocArg, ealloc);
    // -------- //
}

template <class T, class EAlloc, class ...Args>
bool doTestUsesAllocV2(EAlloc const& ealloc, Args&&... args)
{
    PMATest<T> TH;
    // UNDER TEST //
    TH.construct(std::forward<Args>(args)..., ealloc);
    return checkConstruct<Args&&...>(*TH.ptr, UA_AllocLast, ealloc);
    // -------- //
}

template <class Alloc, class ...Args>
void test_pmr_uses_alloc(Args&&... args)
{
    TestResource R(12435);
    ex::memory_resource* M = &R;
    {
        // NotUsesAllocator provides valid signatures for each uses-allocator
        // construction but does not supply the required allocator_type typedef.
        // Test that we can call these constructors manually without
        // polymorphic_allocator interfering.
        using T = NotUsesAllocator<Alloc, sizeof...(Args)>;
        assert(doTestUsesAllocV0<T>(std::forward<Args>(args)...));
        assert((doTestUsesAllocV1<T>(M, std::forward<Args>(args)...)));
        assert((doTestUsesAllocV2<T>(M, std::forward<Args>(args)...)));
    }
    {
        // Test T(std::allocator_arg_t, Alloc const&, Args...) construction
        using T = UsesAllocatorV1<Alloc, sizeof...(Args)>;
        assert((doTest<T>(UA_AllocArg, std::forward<Args>(args)...)));
    }
    {
        // Test T(Args..., Alloc const&) construction
        using T = UsesAllocatorV2<Alloc, sizeof...(Args)>;
        assert((doTest<T>(UA_AllocLast, std::forward<Args>(args)...)));
    }
    {
        // Test that T(std::allocator_arg_t, Alloc const&, Args...) construction
        // is preferred when T(Args..., Alloc const&) is also available.
        using T = UsesAllocatorV3<Alloc, sizeof...(Args)>;
        assert((doTest<T>(UA_AllocArg, std::forward<Args>(args)...)));
    }
}

template <class Alloc, class ...Args>
void test_pmr_not_uses_alloc(Args&&... args)
{
    TestResource R(12435);
    ex::memory_resource* M = &R;
    {
        // NotUsesAllocator provides valid signatures for each uses-allocator
        // construction but does not supply the required allocator_type typedef.
        // Test that we can call these constructors manually without
        // polymorphic_allocator interfering.
        using T = NotUsesAllocator<Alloc, sizeof...(Args)>;
        assert(doTestUsesAllocV0<T>(std::forward<Args>(args)...));
        assert((doTestUsesAllocV1<T>(M, std::forward<Args>(args)...)));
        assert((doTestUsesAllocV2<T>(M, std::forward<Args>(args)...)));
    }
    {
        // Test T(std::allocator_arg_t, Alloc const&, Args...) construction
        using T = UsesAllocatorV1<Alloc, sizeof...(Args)>;
        assert((doTest<T>(UA_None, std::forward<Args>(args)...)));
    }
    {
        // Test T(Args..., Alloc const&) construction
        using T = UsesAllocatorV2<Alloc, sizeof...(Args)>;
        assert((doTest<T>(UA_None, std::forward<Args>(args)...)));
    }
    {
        // Test that T(std::allocator_arg_t, Alloc const&, Args...) construction
        // is preferred when T(Args..., Alloc const&) is also available.
        using T = UsesAllocatorV3<Alloc, sizeof...(Args)>;
        assert((doTest<T>(UA_None, std::forward<Args>(args)...)));
    }
}

// Test that polymorphic_allocator does not prevent us from manually
// doing non-pmr uses-allocator construction.
template <class Alloc, class AllocObj, class ...Args>
void test_non_pmr_uses_alloc(AllocObj const& A, Args&&... args)
{
    {
        using T = NotUsesAllocator<Alloc, sizeof...(Args)>;
        assert(doTestUsesAllocV0<T>(std::forward<Args>(args)...));
        assert((doTestUsesAllocV1<T>(A, std::forward<Args>(args)...)));
        assert((doTestUsesAllocV2<T>(A, std::forward<Args>(args)...)));
    }
    {
        using T = UsesAllocatorV1<Alloc, sizeof...(Args)>;
        assert(doTestUsesAllocV0<T>(std::forward<Args>(args)...));
        assert((doTestUsesAllocV1<T>(A, std::forward<Args>(args)...)));
    }
    {
        using T = UsesAllocatorV2<Alloc, sizeof...(Args)>;
        assert(doTestUsesAllocV0<T>(std::forward<Args>(args)...));
        assert((doTestUsesAllocV2<T>(A, std::forward<Args>(args)...)));
    }
    {
        using T = UsesAllocatorV3<Alloc, sizeof...(Args)>;
        assert(doTestUsesAllocV0<T>(std::forward<Args>(args)...));
        assert((doTestUsesAllocV1<T>(A, std::forward<Args>(args)...)));
        assert((doTestUsesAllocV2<T>(A, std::forward<Args>(args)...)));
    }
}

int main()
{
    using ET = std::experimental::erased_type;
    using PMR = ex::memory_resource*;
    using PMA = ex::polymorphic_allocator<void>;
    using STDA = std::allocator<char>;
    using TESTA = test_allocator<char>;

    int value = 42;
    const int cvalue = 43;
    {
        test_pmr_uses_alloc<ET>();
        test_pmr_not_uses_alloc<PMR>();
        test_pmr_uses_alloc<PMA>();
        test_pmr_uses_alloc<ET>(value);
        test_pmr_not_uses_alloc<PMR>(value);
        test_pmr_uses_alloc<PMA>(value);
        test_pmr_uses_alloc<ET>(cvalue);
        test_pmr_not_uses_alloc<PMR>(cvalue);
        test_pmr_uses_alloc<PMA>(cvalue);
        test_pmr_uses_alloc<ET>(cvalue, std::move(value));
        test_pmr_not_uses_alloc<PMR>(cvalue, std::move(value));
        test_pmr_uses_alloc<PMA>(cvalue, std::move(value));
    }
    {
        STDA std_alloc;
        TESTA test_alloc(42);
        test_non_pmr_uses_alloc<STDA>(std_alloc);
        test_non_pmr_uses_alloc<TESTA>(test_alloc);
        test_non_pmr_uses_alloc<STDA>(std_alloc, value);
        test_non_pmr_uses_alloc<TESTA>(test_alloc, value);
        test_non_pmr_uses_alloc<STDA>(std_alloc, cvalue);
        test_non_pmr_uses_alloc<TESTA>(test_alloc, cvalue);
        test_non_pmr_uses_alloc<STDA>(std_alloc, cvalue, std::move(value));
        test_non_pmr_uses_alloc<TESTA>(test_alloc, cvalue, std::move(value));
    }
}
