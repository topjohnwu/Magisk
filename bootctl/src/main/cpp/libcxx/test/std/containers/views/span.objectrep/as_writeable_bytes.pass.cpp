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

// template <class ElementType, ptrdiff_t Extent>
//     span<byte,
//          Extent == dynamic_extent
//              ? dynamic_extent
//              : static_cast<ptrdiff_t>(sizeof(ElementType)) * Extent>
//     as_writeable_bytes(span<ElementType, Extent> s) noexcept;


#include <span>
#include <cassert>
#include <string>

#include "test_macros.h"

template<typename Span>
void testRuntimeSpan(Span sp)
{
    ASSERT_NOEXCEPT(std::as_writeable_bytes(sp));

    auto spBytes = std::as_writeable_bytes(sp);
    using SB = decltype(spBytes);
    ASSERT_SAME_TYPE(std::byte, typename SB::element_type);

    if (sp.extent == std::dynamic_extent)
        assert(spBytes.extent == std::dynamic_extent);
    else
        assert(spBytes.extent == static_cast<std::ptrdiff_t>(sizeof(typename Span::element_type)) * sp.extent);

    assert(static_cast<void*>(spBytes.data()) == static_cast<void*>(sp.data()));
    assert(spBytes.size() == sp.size_bytes());
}

struct A{};
int iArr2[] = { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9};

int main ()
{
    testRuntimeSpan(std::span<int>        ());
    testRuntimeSpan(std::span<long>       ());
    testRuntimeSpan(std::span<double>     ());
    testRuntimeSpan(std::span<A>          ());
    testRuntimeSpan(std::span<std::string>());

    testRuntimeSpan(std::span<int, 0>        ());
    testRuntimeSpan(std::span<long, 0>       ());
    testRuntimeSpan(std::span<double, 0>     ());
    testRuntimeSpan(std::span<A, 0>          ());
    testRuntimeSpan(std::span<std::string, 0>());

    testRuntimeSpan(std::span<int>(iArr2, 1));
    testRuntimeSpan(std::span<int>(iArr2, 2));
    testRuntimeSpan(std::span<int>(iArr2, 3));
    testRuntimeSpan(std::span<int>(iArr2, 4));
    testRuntimeSpan(std::span<int>(iArr2, 5));

    testRuntimeSpan(std::span<int, 1>(iArr2 + 5, 1));
    testRuntimeSpan(std::span<int, 2>(iArr2 + 4, 2));
    testRuntimeSpan(std::span<int, 3>(iArr2 + 3, 3));
    testRuntimeSpan(std::span<int, 4>(iArr2 + 2, 4));
    testRuntimeSpan(std::span<int, 5>(iArr2 + 1, 5));

    std::string s;
    testRuntimeSpan(std::span<std::string>(&s, (std::ptrdiff_t) 0));
    testRuntimeSpan(std::span<std::string>(&s, 1));
}
