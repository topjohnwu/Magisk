//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <array>

// tuple_element<I, array<T, N> >::type

// Prevent -Warray-bounds from issuing a diagnostic when testing with clang verify.
#if defined(__clang__)
#pragma clang diagnostic ignored "-Warray-bounds"
#endif

#include <array>
#include <cassert>


// std::array is explicitly allowed to be initialized with A a = { init-list };.
// Disable the missing braces warning for this reason.
#include "disable_missing_braces_warning.h"

int main()
{
    {
        typedef double T;
        typedef std::array<T, 3> C;
        std::tuple_element<3, C> foo; // expected-note {{requested here}}
        // expected-error-re@array:* {{static_assert failed{{( due to requirement '3U[L]{0,2} < 3U[L]{0,2}')?}} "Index out of bounds in std::tuple_element<> (std::array)"}}
    }
}
