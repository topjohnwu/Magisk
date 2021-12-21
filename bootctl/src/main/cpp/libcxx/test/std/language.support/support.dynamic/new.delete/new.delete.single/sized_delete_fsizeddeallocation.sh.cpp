//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test sized operator delete replacement.

// Note that sized delete operator definitions below are simply ignored
// when sized deallocation is not supported, e.g., prior to C++14.

// UNSUPPORTED: sanitizer-new-delete
// XFAIL: availability=macosx10.11
// XFAIL: availability=macosx10.10
// XFAIL: availability=macosx10.9
// XFAIL: availability=macosx10.8
// XFAIL: availability=macosx10.7

// NOTE: Only clang-3.7 and GCC 5.1 and greater support -fsized-deallocation.
// REQUIRES: -fsized-deallocation

// RUN: %build -fsized-deallocation -O3
// RUN: %run

#if !defined(__cpp_sized_deallocation)
# error __cpp_sized_deallocation should be defined
#endif

#if !(__cpp_sized_deallocation >= 201309L)
# error expected __cpp_sized_deallocation >= 201309L
#endif

#include <new>
#include <cstddef>
#include <cstdlib>
#include <cassert>

#include "test_macros.h"

int unsized_delete_called = 0;
int unsized_delete_nothrow_called = 0;
int sized_delete_called = 0;

void operator delete(void* p) TEST_NOEXCEPT
{
    ++unsized_delete_called;
    std::free(p);
}

void operator delete(void* p, const std::nothrow_t&) TEST_NOEXCEPT
{
    ++unsized_delete_nothrow_called;
    std::free(p);
}

void operator delete(void* p, std::size_t) TEST_NOEXCEPT
{
    ++sized_delete_called;
    std::free(p);
}

int main()
{
    int *x = new int(42);
    DoNotOptimize(x);
    assert(0 == sized_delete_called);
    assert(0 == unsized_delete_called);
    assert(0 == unsized_delete_nothrow_called);

    delete x;
    DoNotOptimize(x);
    assert(1 == sized_delete_called);
    assert(0 == unsized_delete_called);
    assert(0 == unsized_delete_nothrow_called);
}
