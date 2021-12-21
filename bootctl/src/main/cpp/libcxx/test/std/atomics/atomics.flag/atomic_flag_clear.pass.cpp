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

// void atomic_flag_clear(volatile atomic_flag*);
// void atomic_flag_clear(atomic_flag*);

#include <atomic>
#include <cassert>

int main()
{
    {
        std::atomic_flag f;
        f.clear();
        f.test_and_set();
        atomic_flag_clear(&f);
        assert(f.test_and_set() == 0);
    }
    {
        volatile std::atomic_flag f;
        f.clear();
        f.test_and_set();
        atomic_flag_clear(&f);
        assert(f.test_and_set() == 0);
    }
}
