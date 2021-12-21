//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14, c++17

// TODO: Remove this when filesystem gets integrated into the dylib
// REQUIRES: c++filesystem

// <chrono>

// file_clock

// rep should be signed

#include <chrono>
#include <cassert>

int main()
{
    static_assert(std::is_signed<std::chrono::file_clock::rep>::value, "");
    assert(std::chrono::file_clock::duration::min() <
           std::chrono::file_clock::duration::zero());
}
