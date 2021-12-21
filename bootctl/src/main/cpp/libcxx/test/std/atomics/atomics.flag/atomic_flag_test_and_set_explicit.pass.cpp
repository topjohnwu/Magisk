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

// bool atomic_flag_test_and_set_explicit(volatile atomic_flag*, memory_order);
// bool atomic_flag_test_and_set_explicit(atomic_flag*, memory_order);

#include <atomic>
#include <cassert>

int main()
{
    {
        std::atomic_flag f;
        f.clear();
        assert(atomic_flag_test_and_set_explicit(&f, std::memory_order_relaxed) == 0);
        assert(f.test_and_set() == 1);
    }
    {
        std::atomic_flag f;
        f.clear();
        assert(atomic_flag_test_and_set_explicit(&f, std::memory_order_consume) == 0);
        assert(f.test_and_set() == 1);
    }
    {
        std::atomic_flag f;
        f.clear();
        assert(atomic_flag_test_and_set_explicit(&f, std::memory_order_acquire) == 0);
        assert(f.test_and_set() == 1);
    }
    {
        std::atomic_flag f;
        f.clear();
        assert(atomic_flag_test_and_set_explicit(&f, std::memory_order_release) == 0);
        assert(f.test_and_set() == 1);
    }
    {
        std::atomic_flag f;
        f.clear();
        assert(atomic_flag_test_and_set_explicit(&f, std::memory_order_acq_rel) == 0);
        assert(f.test_and_set() == 1);
    }
    {
        std::atomic_flag f;
        f.clear();
        assert(atomic_flag_test_and_set_explicit(&f, std::memory_order_seq_cst) == 0);
        assert(f.test_and_set() == 1);
    }
    {
        volatile std::atomic_flag f;
        f.clear();
        assert(atomic_flag_test_and_set_explicit(&f, std::memory_order_relaxed) == 0);
        assert(f.test_and_set() == 1);
    }
    {
        volatile std::atomic_flag f;
        f.clear();
        assert(atomic_flag_test_and_set_explicit(&f, std::memory_order_consume) == 0);
        assert(f.test_and_set() == 1);
    }
    {
        volatile std::atomic_flag f;
        f.clear();
        assert(atomic_flag_test_and_set_explicit(&f, std::memory_order_acquire) == 0);
        assert(f.test_and_set() == 1);
    }
    {
        volatile std::atomic_flag f;
        f.clear();
        assert(atomic_flag_test_and_set_explicit(&f, std::memory_order_release) == 0);
        assert(f.test_and_set() == 1);
    }
    {
        volatile std::atomic_flag f;
        f.clear();
        assert(atomic_flag_test_and_set_explicit(&f, std::memory_order_acq_rel) == 0);
        assert(f.test_and_set() == 1);
    }
    {
        volatile std::atomic_flag f;
        f.clear();
        assert(atomic_flag_test_and_set_explicit(&f, std::memory_order_seq_cst) == 0);
        assert(f.test_and_set() == 1);
    }
}
