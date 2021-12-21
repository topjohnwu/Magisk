//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// Verify TEST_WORKAROUND_C1XX_BROKEN_ZA_CTOR_CHECK.

#include <type_traits>

#include "test_workarounds.h"

struct X {
    X(int) {}

    X(X&&) = default;
    X& operator=(X&&) = default;

private:
    X(const X&) = default;
    X& operator=(const X&) = default;
};

void PushFront(X&&) {}

template<class T = int>
auto test(int) -> decltype(PushFront(std::declval<T>()), std::true_type{});
auto test(long) -> std::false_type;

int main() {
#if defined(TEST_WORKAROUND_C1XX_BROKEN_ZA_CTOR_CHECK)
    static_assert(!decltype(test(0))::value, "");
#else
    static_assert(decltype(test(0))::value, "");
#endif
}
