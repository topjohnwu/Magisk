//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <memory>

// default_delete

// Test that default_delete<T[]>'s operator() requires a complete type

#include <memory>
#include <cassert>

struct A;

int main()
{
    std::default_delete<A[]> d;
    A* p = 0;
    d(p);
}
