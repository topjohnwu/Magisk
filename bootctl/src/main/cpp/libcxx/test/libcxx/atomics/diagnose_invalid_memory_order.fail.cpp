//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// This test fails because diagnose_if doesn't emit all of the diagnostics
// when -fdelayed-template-parsing is enabled, like it is on Windows.
// XFAIL: LIBCXX-WINDOWS-FIXME

// REQUIRES: verify-support, diagnose-if-support
// UNSUPPORTED: libcpp-has-no-threads

// <atomic>

// Test that invalid memory order arguments are diagnosed where possible.

#include <atomic>

int main() {
    std::atomic<int> x(42);
    volatile std::atomic<int>& vx = x;
    int val1 = 1; ((void)val1);
    int val2 = 2; ((void)val2);
    // load operations
    {
        x.load(std::memory_order_release); // expected-warning {{memory order argument to atomic operation is invalid}}
        x.load(std::memory_order_acq_rel); // expected-warning {{memory order argument to atomic operation is invalid}}
        vx.load(std::memory_order_release); // expected-warning {{memory order argument to atomic operation is invalid}}
        vx.load(std::memory_order_acq_rel); // expected-warning {{memory order argument to atomic operation is invalid}}
        // valid memory orders
        x.load(std::memory_order_relaxed);
        x.load(std::memory_order_consume);
        x.load(std::memory_order_acquire);
        x.load(std::memory_order_seq_cst);
    }
    {
        std::atomic_load_explicit(&x, std::memory_order_release); // expected-warning {{memory order argument to atomic operation is invalid}}
        std::atomic_load_explicit(&x, std::memory_order_acq_rel); // expected-warning {{memory order argument to atomic operation is invalid}}
        std::atomic_load_explicit(&vx, std::memory_order_release); // expected-warning {{memory order argument to atomic operation is invalid}}
        std::atomic_load_explicit(&vx, std::memory_order_acq_rel); // expected-warning {{memory order argument to atomic operation is invalid}}
        // valid memory orders
        std::atomic_load_explicit(&x, std::memory_order_relaxed);
        std::atomic_load_explicit(&x, std::memory_order_consume);
        std::atomic_load_explicit(&x, std::memory_order_acquire);
        std::atomic_load_explicit(&x, std::memory_order_seq_cst);
    }
    // store operations
    {
        x.store(42, std::memory_order_consume); // expected-warning {{memory order argument to atomic operation is invalid}}
        x.store(42, std::memory_order_acquire); // expected-warning {{memory order argument to atomic operation is invalid}}
        x.store(42, std::memory_order_acq_rel); // expected-warning {{memory order argument to atomic operation is invalid}}
        vx.store(42, std::memory_order_consume); // expected-warning {{memory order argument to atomic operation is invalid}}
        vx.store(42, std::memory_order_acquire); // expected-warning {{memory order argument to atomic operation is invalid}}
        vx.store(42, std::memory_order_acq_rel); // expected-warning {{memory order argument to atomic operation is invalid}}
        // valid memory orders
        x.store(42, std::memory_order_relaxed);
        x.store(42, std::memory_order_release);
        x.store(42, std::memory_order_seq_cst);
    }
    {
        std::atomic_store_explicit(&x, 42, std::memory_order_consume); // expected-warning {{memory order argument to atomic operation is invalid}}
        std::atomic_store_explicit(&x, 42, std::memory_order_acquire); // expected-warning {{memory order argument to atomic operation is invalid}}
        std::atomic_store_explicit(&x, 42, std::memory_order_acq_rel); // expected-warning {{memory order argument to atomic operation is invalid}}
        std::atomic_store_explicit(&vx, 42, std::memory_order_consume); // expected-warning {{memory order argument to atomic operation is invalid}}
        std::atomic_store_explicit(&vx, 42, std::memory_order_acquire); // expected-warning {{memory order argument to atomic operation is invalid}}
        std::atomic_store_explicit(&vx, 42, std::memory_order_acq_rel); // expected-warning {{memory order argument to atomic operation is invalid}}
        // valid memory orders
        std::atomic_store_explicit(&x, 42, std::memory_order_relaxed);
        std::atomic_store_explicit(&x, 42, std::memory_order_release);
        std::atomic_store_explicit(&x, 42, std::memory_order_seq_cst);
    }
    // compare exchange weak
    {
        x.compare_exchange_weak(val1, val2, std::memory_order_seq_cst, std::memory_order_release); // expected-warning {{memory order argument to atomic operation is invalid}}
        x.compare_exchange_weak(val1, val2, std::memory_order_seq_cst, std::memory_order_acq_rel); // expected-warning {{memory order argument to atomic operation is invalid}}
        vx.compare_exchange_weak(val1, val2, std::memory_order_seq_cst, std::memory_order_release); // expected-warning {{memory order argument to atomic operation is invalid}}
        vx.compare_exchange_weak(val1, val2, std::memory_order_seq_cst, std::memory_order_acq_rel); // expected-warning {{memory order argument to atomic operation is invalid}}
        // valid memory orders
        x.compare_exchange_weak(val1, val2, std::memory_order_seq_cst, std::memory_order_relaxed);
        x.compare_exchange_weak(val1, val2, std::memory_order_seq_cst, std::memory_order_consume);
        x.compare_exchange_weak(val1, val2, std::memory_order_seq_cst, std::memory_order_acquire);
        x.compare_exchange_weak(val1, val2, std::memory_order_seq_cst, std::memory_order_seq_cst);
        // Test that the cmpxchg overload with only one memory order argument
        // does not generate any diagnostics.
        x.compare_exchange_weak(val1, val2, std::memory_order_release);
    }
    {
        std::atomic_compare_exchange_weak_explicit(&x, &val1, val2, std::memory_order_seq_cst, std::memory_order_release); // expected-warning {{memory order argument to atomic operation is invalid}}
        std::atomic_compare_exchange_weak_explicit(&x, &val1, val2, std::memory_order_seq_cst, std::memory_order_acq_rel); // expected-warning {{memory order argument to atomic operation is invalid}}
        std::atomic_compare_exchange_weak_explicit(&vx, &val1, val2, std::memory_order_seq_cst, std::memory_order_release); // expected-warning {{memory order argument to atomic operation is invalid}}
        std::atomic_compare_exchange_weak_explicit(&vx, &val1, val2, std::memory_order_seq_cst, std::memory_order_acq_rel); // expected-warning {{memory order argument to atomic operation is invalid}}
        // valid memory orders
        std::atomic_compare_exchange_weak_explicit(&x, &val1, val2, std::memory_order_seq_cst, std::memory_order_relaxed);
        std::atomic_compare_exchange_weak_explicit(&x, &val1, val2, std::memory_order_seq_cst, std::memory_order_consume);
        std::atomic_compare_exchange_weak_explicit(&x, &val1, val2, std::memory_order_seq_cst, std::memory_order_acquire);
        std::atomic_compare_exchange_weak_explicit(&x, &val1, val2, std::memory_order_seq_cst, std::memory_order_seq_cst);
    }
    // compare exchange strong
    {
        x.compare_exchange_strong(val1, val2, std::memory_order_seq_cst, std::memory_order_release); // expected-warning {{memory order argument to atomic operation is invalid}}
        x.compare_exchange_strong(val1, val2, std::memory_order_seq_cst, std::memory_order_acq_rel); // expected-warning {{memory order argument to atomic operation is invalid}}
        vx.compare_exchange_strong(val1, val2, std::memory_order_seq_cst, std::memory_order_release); // expected-warning {{memory order argument to atomic operation is invalid}}
        vx.compare_exchange_strong(val1, val2, std::memory_order_seq_cst, std::memory_order_acq_rel); // expected-warning {{memory order argument to atomic operation is invalid}}
        // valid memory orders
        x.compare_exchange_strong(val1, val2, std::memory_order_seq_cst, std::memory_order_relaxed);
        x.compare_exchange_strong(val1, val2, std::memory_order_seq_cst, std::memory_order_consume);
        x.compare_exchange_strong(val1, val2, std::memory_order_seq_cst, std::memory_order_acquire);
        x.compare_exchange_strong(val1, val2, std::memory_order_seq_cst, std::memory_order_seq_cst);
        // Test that the cmpxchg overload with only one memory order argument
        // does not generate any diagnostics.
        x.compare_exchange_strong(val1, val2, std::memory_order_release);
    }
    {
        std::atomic_compare_exchange_strong_explicit(&x, &val1, val2, std::memory_order_seq_cst, std::memory_order_release); // expected-warning {{memory order argument to atomic operation is invalid}}
        std::atomic_compare_exchange_strong_explicit(&x, &val1, val2, std::memory_order_seq_cst, std::memory_order_acq_rel); // expected-warning {{memory order argument to atomic operation is invalid}}
        std::atomic_compare_exchange_strong_explicit(&vx, &val1, val2, std::memory_order_seq_cst, std::memory_order_release); // expected-warning {{memory order argument to atomic operation is invalid}}
        std::atomic_compare_exchange_strong_explicit(&vx, &val1, val2, std::memory_order_seq_cst, std::memory_order_acq_rel); // expected-warning {{memory order argument to atomic operation is invalid}}
        // valid memory orders
        std::atomic_compare_exchange_strong_explicit(&x, &val1, val2, std::memory_order_seq_cst, std::memory_order_relaxed);
        std::atomic_compare_exchange_strong_explicit(&x, &val1, val2, std::memory_order_seq_cst, std::memory_order_consume);
        std::atomic_compare_exchange_strong_explicit(&x, &val1, val2, std::memory_order_seq_cst, std::memory_order_acquire);
        std::atomic_compare_exchange_strong_explicit(&x, &val1, val2, std::memory_order_seq_cst, std::memory_order_seq_cst);
    }
}
