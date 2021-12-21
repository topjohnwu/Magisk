//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <memory>

// template <ObjectType T> T* addressof(T&& r) = delete;

#include <memory>
#include <cassert>

#include "test_macros.h"

int main()
{
#if TEST_STD_VER > 14
    const int *p = std::addressof<const int>(0);
#else
#error
#endif
}
