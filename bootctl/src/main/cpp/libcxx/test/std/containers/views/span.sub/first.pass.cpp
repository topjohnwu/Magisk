// -*- C++ -*-
//===------------------------------ span ---------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===---------------------------------------------------------------------===//
// UNSUPPORTED: c++98, c++03, c++11, c++14, c++17

// <span>

// template<ptrdiff_t Count>
//  constexpr span<element_type, Count> first() const;
//
// constexpr span<element_type, dynamic_extent> first(index_type count) const;
//
//  Requires: 0 <= Count && Count <= size().


#include <span>
#include <cassert>
#include <algorithm>
#include <string>

#include "test_macros.h"

template <typename Span, ptrdiff_t Count>
constexpr bool testConstexprSpan(Span sp)
{
    LIBCPP_ASSERT((noexcept(sp.template first<Count>())));
    LIBCPP_ASSERT((noexcept(sp.first(Count))));
    auto s1 = sp.template first<Count>();
    auto s2 = sp.first(Count);
    using S1 = decltype(s1);
    using S2 = decltype(s2);
    ASSERT_SAME_TYPE(typename Span::value_type, typename S1::value_type);
    ASSERT_SAME_TYPE(typename Span::value_type, typename S2::value_type);
    static_assert(S1::extent == Count, "");
    static_assert(S2::extent == std::dynamic_extent, "");
    return
        s1.data() == s2.data()
     && s1.size() == s2.size()
     && std::equal(s1.begin(), s1.end(), sp.begin());
}


template <typename Span, ptrdiff_t Count>
void testRuntimeSpan(Span sp)
{
    LIBCPP_ASSERT((noexcept(sp.template first<Count>())));
    LIBCPP_ASSERT((noexcept(sp.first(Count))));
    auto s1 = sp.template first<Count>();
    auto s2 = sp.first(Count);
    using S1 = decltype(s1);
    using S2 = decltype(s2);
    ASSERT_SAME_TYPE(typename Span::value_type, typename S1::value_type);
    ASSERT_SAME_TYPE(typename Span::value_type, typename S2::value_type);
    static_assert(S1::extent == Count, "");
    static_assert(S2::extent == std::dynamic_extent, "");
    assert(s1.data() == s2.data());
    assert(s1.size() == s2.size());
    assert(std::equal(s1.begin(), s1.end(), sp.begin()));
}


constexpr int carr1[] = {1,2,3,4};
          int   arr[] = {5,6,7};
std::string   sarr [] = { "ABC", "DEF", "GHI", "JKL", "MNO"};

int main ()
{
    {
    using Sp = std::span<const int>;
    static_assert(testConstexprSpan<Sp, 0>(Sp{}), "");

    static_assert(testConstexprSpan<Sp, 0>(Sp{carr1}), "");
    static_assert(testConstexprSpan<Sp, 1>(Sp{carr1}), "");
    static_assert(testConstexprSpan<Sp, 2>(Sp{carr1}), "");
    static_assert(testConstexprSpan<Sp, 3>(Sp{carr1}), "");
    static_assert(testConstexprSpan<Sp, 4>(Sp{carr1}), "");
    }

    {
    using Sp = std::span<const int, 4>;

    static_assert(testConstexprSpan<Sp, 0>(Sp{carr1}), "");
    static_assert(testConstexprSpan<Sp, 1>(Sp{carr1}), "");
    static_assert(testConstexprSpan<Sp, 2>(Sp{carr1}), "");
    static_assert(testConstexprSpan<Sp, 3>(Sp{carr1}), "");
    static_assert(testConstexprSpan<Sp, 4>(Sp{carr1}), "");
    }

    {
    using Sp = std::span<int>;
    testRuntimeSpan<Sp, 0>(Sp{});

    testRuntimeSpan<Sp, 0>(Sp{arr});
    testRuntimeSpan<Sp, 1>(Sp{arr});
    testRuntimeSpan<Sp, 2>(Sp{arr});
    testRuntimeSpan<Sp, 3>(Sp{arr});
    }

    {
    using Sp = std::span<int, 3>;

    testRuntimeSpan<Sp, 0>(Sp{arr});
    testRuntimeSpan<Sp, 1>(Sp{arr});
    testRuntimeSpan<Sp, 2>(Sp{arr});
    testRuntimeSpan<Sp, 3>(Sp{arr});
    }

    {
    using Sp = std::span<std::string>;
    testConstexprSpan<Sp, 0>(Sp{});

    testRuntimeSpan<Sp, 0>(Sp{sarr});
    testRuntimeSpan<Sp, 1>(Sp{sarr});
    testRuntimeSpan<Sp, 2>(Sp{sarr});
    testRuntimeSpan<Sp, 3>(Sp{sarr});
    testRuntimeSpan<Sp, 4>(Sp{sarr});
    testRuntimeSpan<Sp, 5>(Sp{sarr});
    }

    {
    using Sp = std::span<std::string, 5>;

    testRuntimeSpan<Sp, 0>(Sp{sarr});
    testRuntimeSpan<Sp, 1>(Sp{sarr});
    testRuntimeSpan<Sp, 2>(Sp{sarr});
    testRuntimeSpan<Sp, 3>(Sp{sarr});
    testRuntimeSpan<Sp, 4>(Sp{sarr});
    testRuntimeSpan<Sp, 5>(Sp{sarr});
    }
}
