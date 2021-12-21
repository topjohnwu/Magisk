//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <valarray>

// template <class T> class mask_array

// mask_array() = delete;

#include <valarray>
#include <type_traits>

int main()
{
    std::mask_array<int> s;
}
