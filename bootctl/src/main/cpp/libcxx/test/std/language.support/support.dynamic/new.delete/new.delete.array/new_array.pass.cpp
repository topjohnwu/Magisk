//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test operator new[]
// NOTE: asan and msan will not call the new handler.
// UNSUPPORTED: sanitizer-new-delete


#include <new>
#include <cstddef>
#include <cassert>
#include <limits>

#include "test_macros.h"

int new_handler_called = 0;

void my_new_handler()
{
    ++new_handler_called;
    std::set_new_handler(0);
}

int A_constructed = 0;

struct A
{
    A() {++A_constructed;}
    ~A() {--A_constructed;}
};

int main()
{
#ifndef TEST_HAS_NO_EXCEPTIONS
    std::set_new_handler(my_new_handler);
    try
    {
        void* vp = operator new[] (std::numeric_limits<std::size_t>::max());
        DoNotOptimize(vp);
        assert(false);
    }
    catch (std::bad_alloc&)
    {
        assert(new_handler_called == 1);
    }
    catch (...)
    {
        assert(false);
    }
#endif
    A* ap = new A[3];
    DoNotOptimize(ap);
    assert(ap);
    assert(A_constructed == 3);
    delete [] ap;
    DoNotOptimize(ap);
    assert(A_constructed == 0);
}
