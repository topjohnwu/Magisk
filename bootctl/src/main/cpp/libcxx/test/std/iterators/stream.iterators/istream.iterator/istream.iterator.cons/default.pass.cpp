//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Usage of is_trivially_constructible is broken with these compilers.
// See https://bugs.llvm.org/show_bug.cgi?id=31016
// XFAIL: clang-3.7, apple-clang-7 && c++17

// <iterator>

// class istream_iterator

// constexpr istream_iterator();
// C++17 says: If is_trivially_default_constructible_v<T> is true, then this
//    constructor is a constexpr constructor.

#include <iterator>
#include <cassert>
#include <string>

#include "test_macros.h"

struct S { S(); }; // not constexpr

#if TEST_STD_VER > 14
template <typename T, bool isTrivial = std::is_trivially_default_constructible_v<T>>
struct test_trivial {
void operator ()() const {
    constexpr std::istream_iterator<T> it;
    (void)it;
    }
};

template <typename T>
struct test_trivial<T, false> {
void operator ()() const {}
};
#endif


int main()
{
    {
    typedef std::istream_iterator<int> T;
    T it;
    assert(it == T());
#if TEST_STD_VER >= 11
    constexpr T it2;
    (void)it2;
#endif
    }

#if TEST_STD_VER > 14
    test_trivial<int>()();
    test_trivial<char>()();
    test_trivial<double>()();
    test_trivial<S>()();
    test_trivial<std::string>()();
#endif
}
