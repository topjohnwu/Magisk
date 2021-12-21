//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <chrono>

// duration

// Test nested types

// typedef Rep rep;
// typedef Period period;

#include <chrono>
#include <type_traits>

int main()
{
    typedef std::chrono::duration<long, std::ratio<3, 2> > D;
    static_assert((std::is_same<D::rep, long>::value), "");
    static_assert((std::is_same<D::period, std::ratio<3, 2> >::value), "");
}
