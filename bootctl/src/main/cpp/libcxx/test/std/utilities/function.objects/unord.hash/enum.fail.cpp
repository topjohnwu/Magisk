//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>

//  Hashing a struct w/o a defined hash should fail.

#include <functional>
#include <cassert>
#include <type_traits>

struct X {};

int main()
{
    X x;
    size_t h = std::hash<X>{} ( x );
}
