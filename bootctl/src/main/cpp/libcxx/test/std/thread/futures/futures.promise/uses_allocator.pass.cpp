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

// <future>

// class promise<R>

// template <class R, class Alloc>
//   struct uses_allocator<promise<R>, Alloc>
//      : true_type { };

#include <future>
#include "test_allocator.h"

int main()
{
    static_assert((std::uses_allocator<std::promise<int>, test_allocator<int> >::value), "");
    static_assert((std::uses_allocator<std::promise<int&>, test_allocator<int> >::value), "");
    static_assert((std::uses_allocator<std::promise<void>, test_allocator<void> >::value), "");
}
