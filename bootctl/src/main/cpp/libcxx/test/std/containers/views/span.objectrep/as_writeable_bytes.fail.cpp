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

const int iArr2[] = { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9};

struct A {};

int main ()
{
    std::as_writeable_bytes(std::span<const int>());            // expected-error {{no matching function for call to 'as_writeable_bytes'}}
    std::as_writeable_bytes(std::span<const long>());           // expected-error {{no matching function for call to 'as_writeable_bytes'}}
    std::as_writeable_bytes(std::span<const double>());         // expected-error {{no matching function for call to 'as_writeable_bytes'}}
    std::as_writeable_bytes(std::span<const A>());              // expected-error {{no matching function for call to 'as_writeable_bytes'}}
    std::as_writeable_bytes(std::span<const std::string>());    // expected-error {{no matching function for call to 'as_writeable_bytes'}}

    std::as_writeable_bytes(std::span<const int, 0>());         // expected-error {{no matching function for call to 'as_writeable_bytes'}}
    std::as_writeable_bytes(std::span<const long, 0>());        // expected-error {{no matching function for call to 'as_writeable_bytes'}}
    std::as_writeable_bytes(std::span<const double, 0>());      // expected-error {{no matching function for call to 'as_writeable_bytes'}}
    std::as_writeable_bytes(std::span<const A, 0>());           // expected-error {{no matching function for call to 'as_writeable_bytes'}}
    std::as_writeable_bytes(std::span<const std::string, 0>()); // expected-error {{no matching function for call to 'as_writeable_bytes'}}

    std::as_writeable_bytes(std::span<const int>   (iArr2, 1));     // expected-error {{no matching function for call to 'as_writeable_bytes'}}
    std::as_writeable_bytes(std::span<const int, 1>(iArr2 + 5, 1)); // expected-error {{no matching function for call to 'as_writeable_bytes'}}
}
