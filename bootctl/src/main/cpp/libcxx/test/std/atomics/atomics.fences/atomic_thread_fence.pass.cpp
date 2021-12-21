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

// void atomic_thread_fence(memory_order m);

#include <atomic>

int main()
{
    std::atomic_thread_fence(std::memory_order_seq_cst);
}
