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

// void atomic_flag_clear_explicit(volatile atomic_flag*, memory_order);
// void atomic_flag_clear_explicit(atomic_flag*, memory_order);

#include <atomic>
#include <cassert>

int main()
{
    {
        std::atomic_flag f; // uninitialized first
        atomic_flag_clear_explicit(&f, std::memory_order_relaxed);
        assert(f.test_and_set() == 0);
        atomic_flag_clear_explicit(&f, std::memory_order_relaxed);
        assert(f.test_and_set() == 0);
    }
    {
        std::atomic_flag f;
        atomic_flag_clear_explicit(&f, std::memory_order_release);
        assert(f.test_and_set() == 0);
        atomic_flag_clear_explicit(&f, std::memory_order_release);
        assert(f.test_and_set() == 0);
    }
    {
        std::atomic_flag f;
        atomic_flag_clear_explicit(&f, std::memory_order_seq_cst);
        assert(f.test_and_set() == 0);
        atomic_flag_clear_explicit(&f, std::memory_order_seq_cst);
        assert(f.test_and_set() == 0);
    }
    {
        volatile std::atomic_flag f;
        atomic_flag_clear_explicit(&f, std::memory_order_relaxed);
        assert(f.test_and_set() == 0);
        atomic_flag_clear_explicit(&f, std::memory_order_relaxed);
        assert(f.test_and_set() == 0);
    }
    {
        volatile std::atomic_flag f;
        atomic_flag_clear_explicit(&f, std::memory_order_release);
        assert(f.test_and_set() == 0);
        atomic_flag_clear_explicit(&f, std::memory_order_release);
        assert(f.test_and_set() == 0);
    }
    {
        volatile std::atomic_flag f;
        atomic_flag_clear_explicit(&f, std::memory_order_seq_cst);
        assert(f.test_and_set() == 0);
        atomic_flag_clear_explicit(&f, std::memory_order_seq_cst);
        assert(f.test_and_set() == 0);
    }
}
