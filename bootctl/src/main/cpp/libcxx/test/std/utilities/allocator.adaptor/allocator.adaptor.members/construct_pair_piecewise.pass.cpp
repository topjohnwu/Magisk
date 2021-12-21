//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <scoped_allocator>

// template <class OtherAlloc, class ...InnerAlloc>
//   class scoped_allocator_adaptor

// template <class U1, class U2, class ...Args1, class ...Args2>
// void scoped_allocator_adaptor::construct(pair<U1, U2>*,
//     piecewise_construct_t, tuple<Args1...>, tuple<Args2...>)

#include <scoped_allocator>
#include <type_traits>
#include <utility>
#include <tuple>
#include <cassert>
#include <cstdlib>
#include "uses_alloc_types.hpp"
#include "controlled_allocators.hpp"


void test_no_inner_alloc()
{
    using VoidAlloc = CountingAllocator<void>;
    AllocController P;
    {
        using T = UsesAllocatorV1<VoidAlloc, 1>;
        using U = UsesAllocatorV2<VoidAlloc, 1>;
        using Pair = std::pair<T, U>;
        int x = 42;
        const int y = 101;
        using Alloc = CountingAllocator<Pair>;
        using SA = std::scoped_allocator_adaptor<Alloc>;
        static_assert(std::uses_allocator<T, CountingAllocator<T> >::value, "");
        Pair * ptr = (Pair*)std::malloc(sizeof(Pair));
        assert(ptr != nullptr);
        Alloc CA(P);
        SA A(CA);
        A.construct(ptr, std::piecewise_construct,
                    std::forward_as_tuple(x),
                    std::forward_as_tuple(std::move(y)));
        assert(checkConstruct<int&>(ptr->first, UA_AllocArg, CA));
        assert(checkConstruct<int const&&>(ptr->second, UA_AllocLast, CA));
        assert((P.checkConstruct<std::piecewise_construct_t const&,
                                 std::tuple<std::allocator_arg_t, SA&, int&>&&,
                                 std::tuple<int const&&, SA&>&&
              >(CA, ptr)));
        A.destroy(ptr);
        std::free(ptr);

    }
    P.reset();
    {
        using T = UsesAllocatorV3<VoidAlloc, 1>;
        using U = NotUsesAllocator<VoidAlloc, 1>;
        using Pair = std::pair<T, U>;
        int x = 42;
        const int y = 101;
        using Alloc = CountingAllocator<Pair>;
        using SA = std::scoped_allocator_adaptor<Alloc>;
        static_assert(std::uses_allocator<T, CountingAllocator<T> >::value, "");
        Pair * ptr = (Pair*)std::malloc(sizeof(Pair));
        assert(ptr != nullptr);
        Alloc CA(P);
        SA A(CA);
        A.construct(ptr, std::piecewise_construct,
                    std::forward_as_tuple(std::move(x)),
                    std::forward_as_tuple(y));
        assert(checkConstruct<int&&>(ptr->first, UA_AllocArg, CA));
        assert(checkConstruct<int const&>(ptr->second, UA_None));
        assert((P.checkConstruct<std::piecewise_construct_t const&,
                                 std::tuple<std::allocator_arg_t, SA&, int&&>&&,
                                 std::tuple<int const&>&&
                   >(CA, ptr)));
        A.destroy(ptr);
        std::free(ptr);
    }
}

void test_with_inner_alloc()
{
    using VoidAlloc2 = CountingAllocator<void, 2>;

    AllocController POuter;
    AllocController PInner;
    {
        using T = UsesAllocatorV1<VoidAlloc2, 1>;
        using U = UsesAllocatorV2<VoidAlloc2, 1>;
        using Pair = std::pair<T, U>;
        int x = 42;
        int y = 101;
        using Outer = CountingAllocator<Pair, 1>;
        using Inner = CountingAllocator<Pair, 2>;
        using SA = std::scoped_allocator_adaptor<Outer, Inner>;
        using SAInner = std::scoped_allocator_adaptor<Inner>;
        static_assert(!std::uses_allocator<T, Outer>::value, "");
        static_assert(std::uses_allocator<T, Inner>::value, "");
        Pair * ptr = (Pair*)std::malloc(sizeof(Pair));
        assert(ptr != nullptr);
        Outer O(POuter);
        Inner I(PInner);
        SA A(O, I);
        A.construct(ptr, std::piecewise_construct,
                    std::forward_as_tuple(x),
                    std::forward_as_tuple(std::move(y)));
        assert(checkConstruct<int&>(ptr->first, UA_AllocArg, I));
        assert(checkConstruct<int &&>(ptr->second, UA_AllocLast));
        assert((POuter.checkConstruct<std::piecewise_construct_t const&,
                                 std::tuple<std::allocator_arg_t, SAInner&, int&>&&,
                                 std::tuple<int &&, SAInner&>&&
              >(O, ptr)));
        A.destroy(ptr);
        std::free(ptr);
    }
    PInner.reset();
    POuter.reset();
    {
        using T = UsesAllocatorV3<VoidAlloc2, 1>;
        using U = NotUsesAllocator<VoidAlloc2, 1>;
        using Pair = std::pair<T, U>;
        int x = 42;
        const int y = 101;
        using Outer = CountingAllocator<Pair, 1>;
        using Inner = CountingAllocator<Pair, 2>;
        using SA = std::scoped_allocator_adaptor<Outer, Inner>;
        using SAInner = std::scoped_allocator_adaptor<Inner>;
        static_assert(!std::uses_allocator<T, Outer>::value, "");
        static_assert(std::uses_allocator<T, Inner>::value, "");
        Pair * ptr = (Pair*)std::malloc(sizeof(Pair));
        assert(ptr != nullptr);
        Outer O(POuter);
        Inner I(PInner);
        SA A(O, I);
        A.construct(ptr, std::piecewise_construct,
                    std::forward_as_tuple(std::move(x)),
                    std::forward_as_tuple(std::move(y)));
        assert(checkConstruct<int&&>(ptr->first, UA_AllocArg, I));
        assert(checkConstruct<int const&&>(ptr->second, UA_None));
        assert((POuter.checkConstruct<std::piecewise_construct_t const&,
                                 std::tuple<std::allocator_arg_t, SAInner&, int&&>&&,
                                 std::tuple<int const&&>&&
              >(O, ptr)));
        A.destroy(ptr);
        std::free(ptr);
    }
}
int main() {
    test_no_inner_alloc();
    test_with_inner_alloc();
}
