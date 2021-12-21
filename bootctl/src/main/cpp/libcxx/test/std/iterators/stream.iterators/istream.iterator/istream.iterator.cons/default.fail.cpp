//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <iterator>

// class istream_iterator

// constexpr istream_iterator();

#include <iterator>
#include <cassert>

#include "test_macros.h"

struct S { S(); }; // not constexpr

int main()
{
#if TEST_STD_VER >= 11
    {
    constexpr std::istream_iterator<S> it;
    }
#else
#error "C++11 only test"
#endif
}
