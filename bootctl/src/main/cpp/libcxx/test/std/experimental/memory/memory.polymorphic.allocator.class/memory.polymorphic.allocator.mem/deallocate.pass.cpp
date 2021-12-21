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

// T* polymorphic_allocator<T>::deallocate(T*, size_t size)

#include <experimental/memory_resource>
#include <type_traits>
#include <cassert>

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
        assert(R.checkDealloc(ret, N * sizeof(T), alignof(T)));

        R.reset();
    }
}

int main()
{
    {
        ex::polymorphic_allocator<int> a;
        static_assert(
            std::is_same<decltype(a.deallocate(nullptr, 0)), void>::value, "");
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
}
