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

// constexpr span() noexcept;
//
//  Remarks: This constructor shall not participate in overload resolution
//          unless Extent <= 0 is true.


#include <span>
#include <cassert>
#include <string>

#include "test_macros.h"

int main ()
{
    std::span<int, 2> s; // expected-error-re@span:* {{static_assert failed{{( due to requirement '2[LL]{0,2} == 0')?}} "Can't default construct a statically sized span with size > 0"}}

//  TODO: This is what I want:
// eXpected-error {{no matching constructor for initialization of 'std::span<int, 2>'}}
}
