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

// <mutex>

// class mutex;

// mutex();

#include <mutex>
#include <type_traits>

int main()
{
    static_assert(std::is_nothrow_default_constructible<std::mutex>::value, "");
    std::mutex m;
}
