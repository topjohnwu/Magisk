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

// This test is marked XFAIL and not UNSUPPORTED because the non-variadic
// declaration of packaged_task is available in C++03. Therefore the test
// should fail because the static_assert fires and not because std::packaged_task
// in undefined.
// XFAIL: c++98, c++03

// <future>
// REQUIRES: c++11 || c++14
// packaged_task allocator support was removed in C++17 (LWG 2976)

// class packaged_task<R(ArgTypes...)>

// template <class Callable, class Alloc>
//   struct uses_allocator<packaged_task<Callable>, Alloc>
//      : true_type { };

#include <future>
#include "test_allocator.h"

int main()
{
    static_assert((std::uses_allocator<std::packaged_task<double(int, char)>, test_allocator<int> >::value), "");
}
