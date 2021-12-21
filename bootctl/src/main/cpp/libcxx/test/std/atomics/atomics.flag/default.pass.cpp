//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// UNSUPPORTED: libcpp-has-no-threads

// <atomic>

// struct atomic_flag

// atomic_flag() = default;

#include <atomic>
#include <new>
#include <cassert>

#include "test_macros.h"

int main()
{
    std::atomic_flag f;
    f.clear();
    assert(f.test_and_set() == 0);
    {
        typedef std::atomic_flag A;
        TEST_ALIGNAS_TYPE(A) char storage[sizeof(A)] = {1};
        A& zero = *new (storage) A();
        assert(!zero.test_and_set());
        zero.~A();
    }
}
