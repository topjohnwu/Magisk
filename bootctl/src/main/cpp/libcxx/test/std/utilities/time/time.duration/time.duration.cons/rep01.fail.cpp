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

// template <class Rep2>
//   explicit duration(const Rep2& r);

// test for explicit

#include <chrono>

#include "../../rep.h"

int main()
{
    std::chrono::duration<int> d = 1;
}
