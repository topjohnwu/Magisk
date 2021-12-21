//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// type_traits

// aligned_union<size_t Len, class ...Types>

#include <type_traits>

class A; // Incomplete

int main()
{
    typedef std::aligned_union<10, A>::type T1;
}
