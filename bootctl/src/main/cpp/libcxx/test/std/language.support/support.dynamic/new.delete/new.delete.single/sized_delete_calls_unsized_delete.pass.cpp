//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test sized operator delete calls the unsized operator delete.
// When sized operator delete is not available (ex C++11) then the unsized
// operator delete is called directly.

// UNSUPPORTED: sanitizer-new-delete

#include <new>
#include <cstddef>
#include <cstdlib>
#include <cassert>

#include "test_macros.h"

int delete_called = 0;
int delete_nothrow_called = 0;

void operator delete(void* p) TEST_NOEXCEPT
{
    ++delete_called;
    std::free(p);
}

void operator delete(void* p, const std::nothrow_t&) TEST_NOEXCEPT
{
    ++delete_nothrow_called;
    std::free(p);
}

int main()
{
    int *x = new int(42);
    DoNotOptimize(x);
    assert(0 == delete_called);
    assert(0 == delete_nothrow_called);

    delete x;
    DoNotOptimize(x);
    assert(1 == delete_called);
    assert(0 == delete_nothrow_called);
}
