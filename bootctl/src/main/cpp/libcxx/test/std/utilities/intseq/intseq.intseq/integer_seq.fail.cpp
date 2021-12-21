//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <utility>

// template<class T, T... I>
// struct integer_sequence
// {
//     typedef T type;
//
//     static constexpr size_t size() noexcept;
// };

// This test is a conforming extension.  The extension turns undefined behavior
//  into a compile-time error.

#include <utility>

#include "test_macros.h"

int main()
{
#if TEST_STD_VER > 11

//  Should fail to compile, since float is not an integral type
    using floatmix = std::integer_sequence<float>;
    floatmix::value_type I;

#else

X

#endif  // TEST_STD_VER > 11
}
