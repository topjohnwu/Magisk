//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <chrono>

// system_clock

// rep should be signed

#include <chrono>
#include <cassert>

int main()
{
    assert(std::chrono::system_clock::duration::min() <
           std::chrono::system_clock::duration::zero());
}
