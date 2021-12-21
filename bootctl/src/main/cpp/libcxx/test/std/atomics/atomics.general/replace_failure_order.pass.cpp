//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: libcpp-has-no-threads

// This test verifies behavior specified by [atomics.types.operations.req]/21:
//
//     When only one memory_order argument is supplied, the value of success is
//     order, and the value of failure is order except that a value of
//     memory_order_acq_rel shall be replaced by the value memory_order_acquire
//     and a value of memory_order_release shall be replaced by the value
//     memory_order_relaxed.
//
// Clang's atomic intrinsics do this for us, but GCC's do not. We don't actually
// have visibility to see what these memory orders are lowered to, but we can at
// least check that they are lowered at all (otherwise there is a compile
// failure with GCC).

#include <atomic>

int main() {
    std::atomic<int> i;
    volatile std::atomic<int> v;
    int exp = 0;

    i.compare_exchange_weak(exp, 0, std::memory_order_acq_rel);
    i.compare_exchange_weak(exp, 0, std::memory_order_release);
    i.compare_exchange_strong(exp, 0, std::memory_order_acq_rel);
    i.compare_exchange_strong(exp, 0, std::memory_order_release);

    v.compare_exchange_weak(exp, 0, std::memory_order_acq_rel);
    v.compare_exchange_weak(exp, 0, std::memory_order_release);
    v.compare_exchange_strong(exp, 0, std::memory_order_acq_rel);
    v.compare_exchange_strong(exp, 0, std::memory_order_release);

    return 0;
}
