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

// template <class T, class ...Args>
// void scoped_allocator_adaptor::construct(T*, Args&&...)

#include <scoped_allocator>
#include <type_traits>
#include <utility>
#include <tuple>
#include <cassert>
#include <cstdlib>
#include "uses_alloc_types.hpp"
#include "controlled_allocators.hpp"

// - If uses_allocator_v<T, inner_allocator_type> is false and
//   is_constructible_v<T, Args...> is true, calls
//   OUTERMOST_ALLOC_TRAITS(*this)::construct(
//      OUTERMOST (*this), p, std::forward<Args>(args)...).
void test_bullet_one() {
    using VoidAlloc2 = CountingAllocator<void, 2>;

    AllocController POuter;
    AllocController PInner;
    {
        using T = NotUsesAllocator<VoidAlloc2, 3>;
        using Outer = CountingAllocator<T, 1>;
        using Inner = CountingAllocator<T, 2>;
        using SA = std::scoped_allocator_adaptor<Outer, Inner>;
        static_assert(!std::uses_allocator<T, Outer>::value, "");
        static_assert(!std::uses_allocator<T, Inner>::value, "");
        T* ptr = (T*)::operator new(sizeof(T));
        Outer O(POuter);
        Inner I(PInner);
        SA A(O, I);
        int x = 42;
        int const& cx = x;
        A.construct(ptr, x, cx, std::move(x));
        assert((checkConstruct<int&, int const&, int&&>(*ptr, UA_None)));
        assert((POuter.checkConstruct<int&, int const&, int&&>(O, ptr)));
        A.destroy(ptr);
        ::operator delete((void*)ptr);
    }
    PInner.reset();
    POuter.reset();
}


// Otherwise, if uses_allocator_v<T, inner_allocator_type> is true and
// is_constructible_v<T, allocator_arg_t, inner_allocator_type&, Args...> is
// true, calls OUTERMOST_ALLOC_TRAITS(*this)::construct(OUTERMOST (*this), p,
//     allocator_arg, inner_allocator(), std::forward<Args>(args)...).
void test_bullet_two() {
    using VoidAlloc2 = CountingAllocator<void, 2>;

    AllocController POuter;
    AllocController PInner;
    {
        using T = UsesAllocatorV1<VoidAlloc2, 3>;
        using Outer = CountingAllocator<T, 1>;
        using Inner = CountingAllocator<T, 2>;
        using SA = std::scoped_allocator_adaptor<Outer, Inner>;
        static_assert(!std::uses_allocator<T, Outer>::value, "");
        static_assert(std::uses_allocator<T, Inner>::value, "");
        T* ptr = (T*)::operator new(sizeof(T));
        Outer O(POuter);
        Inner I(PInner);
        SA A(O, I);
        int x = 42;
        int const& cx = x;
        A.construct(ptr, x, cx, std::move(x));
        assert((checkConstruct<int&, int const&, int&&>(*ptr, UA_AllocArg, I)));
        assert((POuter.checkConstruct<std::allocator_arg_t const&,
                   SA::inner_allocator_type&, int&, int const&, int&&>(O, ptr)));
        A.destroy(ptr);
        ::operator delete((void*)ptr);
    }
    PInner.reset();
    POuter.reset();
}

// Otherwise, if uses_allocator_v<T, inner_allocator_type> is true and
// is_constructible_v<T, Args..., inner_allocator_type&> is true, calls
// OUTERMOST_ALLOC_TRAITS(*this)::construct(OUTERMOST (*this), p,
//   std::forward<Args>(args)..., inner_allocator()).
void test_bullet_three() {
    using VoidAlloc2 = CountingAllocator<void, 2>;

    AllocController POuter;
    AllocController PInner;
    {
        using T = UsesAllocatorV2<VoidAlloc2, 3>;
        using Outer = CountingAllocator<T, 1>;
        using Inner = CountingAllocator<T, 2>;
        using SA = std::scoped_allocator_adaptor<Outer, Inner>;
        static_assert(!std::uses_allocator<T, Outer>::value, "");
        static_assert(std::uses_allocator<T, Inner>::value, "");
        T* ptr = (T*)::operator new(sizeof(T));
        Outer O(POuter);
        Inner I(PInner);
        SA A(O, I);
        int x = 42;
        int const& cx = x;
        A.construct(ptr, x, cx, std::move(x));
        assert((checkConstruct<int&, int const&, int&&>(*ptr, UA_AllocLast, I)));
        assert((POuter.checkConstruct<
                   int&, int const&, int&&,
                   SA::inner_allocator_type&>(O, ptr)));
        A.destroy(ptr);
        ::operator delete((void*)ptr);
    }
    PInner.reset();
    POuter.reset();
}

int main() {
    test_bullet_one();
    test_bullet_two();
    test_bullet_three();
}
