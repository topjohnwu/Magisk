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

// enum class future_status
// {
//     ready,
//     timeout,
//     deferred
// };

#include <future>

int main()
{
    static_assert(static_cast<int>(std::future_status::ready) == 0, "");
    static_assert(static_cast<int>(std::future_status::timeout) == 1, "");
    static_assert(static_cast<int>(std::future_status::deferred) == 2, "");
}
