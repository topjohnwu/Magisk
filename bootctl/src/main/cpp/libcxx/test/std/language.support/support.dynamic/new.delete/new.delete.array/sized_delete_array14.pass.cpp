//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test sized operator delete[] replacement.

// UNSUPPORTED: sanitizer-new-delete, c++98, c++03, c++11

// NOTE: Clang does not enable sized-deallocation in C++14 and beyond by
// default. It is only enabled when -fsized-deallocation is given.
// (except clang-3.6 which temporarily enabled sized-deallocation)
// UNSUPPORTED: clang, apple-clang

// NOTE: GCC 4.9.1 does not support sized-deallocation in C++14. However
// GCC 5.1 does.
// XFAIL: gcc-4.7, gcc-4.8, gcc-4.9

#include <new>
#include <cstddef>
#include <cstdlib>
#include <cassert>

#include "test_macros.h"

int unsized_delete_called = 0;
int unsized_delete_nothrow_called = 0;
int sized_delete_called = 0;

void operator delete[](void* p) TEST_NOEXCEPT
{
    ++unsized_delete_called;
    std::free(p);
}

void operator delete[](void* p, const std::nothrow_t&) TEST_NOEXCEPT
{
    ++unsized_delete_nothrow_called;
    std::free(p);
}

void operator delete[](void* p, std::size_t) TEST_NOEXCEPT
{
    ++sized_delete_called;
    std::free(p);
}

// NOTE: Use a class with a non-trivial destructor as the test type in order
// to ensure the correct overload is called.
// C++14 5.3.5 [expr.delete]p10
// - If the type is complete and if, for the second alternative (delete array)
//   only, the operand is a pointer to a class type with a non-trivial
//   destructor or a (possibly multi-dimensional) array thereof, the function
//   with two parameters is selected.
// - Otherwise, it is unspecified which of the two deallocation functions is
//   selected.
struct A { ~A() {} };

int main()
{
    A* x = new A[3];
    assert(0 == unsized_delete_called);
    assert(0 == unsized_delete_nothrow_called);
    assert(0 == sized_delete_called);

    delete [] x;
    assert(0 == unsized_delete_called);
    assert(0 == unsized_delete_nothrow_called);
    assert(1 == sized_delete_called);
}
