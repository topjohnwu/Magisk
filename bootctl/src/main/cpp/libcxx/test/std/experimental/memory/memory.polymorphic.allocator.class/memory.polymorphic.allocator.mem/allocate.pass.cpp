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

// T* polymorphic_allocator<T>::allocate(size_t n)

#include <experimental/memory_resource>
#include <limits>
#include <memory>
#include <exception>
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "test_memory_resource.hpp"

namespace ex = std::experimental::pmr;

template <size_t S, size_t Align>
void testForSizeAndAlign() {
    using T = typename std::aligned_storage<S, Align>::type;
    TestResource R;
    ex::polymorphic_allocator<T> a(&R);

    for (int N = 1; N <= 5; ++N) {
        auto ret = a.allocate(N);
        assert(R.checkAlloc(ret, N * sizeof(T), alignof(T)));

        a.deallocate(ret, N);
        R.reset();
    }
}

#ifndef TEST_HAS_NO_EXCEPTIONS
template <size_t S>
void testAllocForSizeThrows() {
    using T = typename std::aligned_storage<S>::type;
    using Alloc = ex::polymorphic_allocator<T>;
    using Traits = std::allocator_traits<Alloc>;
    NullResource R;
    Alloc a(&R);

    // Test that allocating exactly the max size does not throw.
    size_t maxSize = Traits::max_size(a);
    try {
        a.allocate(maxSize);
    } catch (...) {
        assert(false);
    }

    size_t sizeTypeMax = std::numeric_limits<std::size_t>::max();
    if (maxSize != sizeTypeMax)
    {
        // Test that allocating size_t(~0) throws bad alloc.
        try {
            a.allocate(sizeTypeMax);
            assert(false);
        } catch (std::exception const&) {
        }

        // Test that allocating even one more than the max size does throw.
        size_t overSize = maxSize + 1;
        try {
            a.allocate(overSize);
            assert(false);
        } catch (std::exception const&) {
        }
    }
}
#endif // TEST_HAS_NO_EXCEPTIONS

int main()
{
    {
        ex::polymorphic_allocator<int> a;
        static_assert(std::is_same<decltype(a.allocate(0)), int*>::value, "");
        static_assert(!noexcept(a.allocate(0)), "");
    }
    {
        constexpr std::size_t MA = alignof(std::max_align_t);
        testForSizeAndAlign<1, 1>();
        testForSizeAndAlign<1, 2>();
        testForSizeAndAlign<1, MA>();
        testForSizeAndAlign<2, 2>();
        testForSizeAndAlign<73, alignof(void*)>();
        testForSizeAndAlign<73, MA>();
        testForSizeAndAlign<13, MA>();
    }
#ifndef TEST_HAS_NO_EXCEPTIONS
    {
        testAllocForSizeThrows<1>();
        testAllocForSizeThrows<2>();
        testAllocForSizeThrows<4>();
        testAllocForSizeThrows<8>();
        testAllocForSizeThrows<16>();
        testAllocForSizeThrows<73>();
        testAllocForSizeThrows<13>();
    }
#endif
}
