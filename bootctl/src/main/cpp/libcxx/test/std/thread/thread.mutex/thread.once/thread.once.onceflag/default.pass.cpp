//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <mutex>

// struct once_flag;

// constexpr once_flag() noexcept;

#include <mutex>
#include "test_macros.h"

int main()
{
    {
    std::once_flag f;
    (void)f;
    }
#if TEST_STD_VER >= 11
    {
    constexpr std::once_flag f;
    (void)f;
    }
#endif
}
