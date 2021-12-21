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

// UNSUPPORTED: c++98, c++03
// Libc++'s enum class emulation does not allow static_cast<Enum>(0) to work.

// <future>

// enum class future_errc
// {
//     broken_promise = implementation-defined,
//     future_already_retrieved = implementation-defined,
//     promise_already_satisfied = implementation-defined,
//     no_state = implementation-defined
// };

#include <future>

int main()
{
    static_assert(std::future_errc::broken_promise != std::future_errc::future_already_retrieved, "");
    static_assert(std::future_errc::broken_promise != std::future_errc::promise_already_satisfied, "");
    static_assert(std::future_errc::broken_promise != std::future_errc::no_state, "");
    static_assert(std::future_errc::future_already_retrieved != std::future_errc::promise_already_satisfied, "");
    static_assert(std::future_errc::future_already_retrieved != std::future_errc::no_state, "");
    static_assert(std::future_errc::promise_already_satisfied != std::future_errc::no_state, "");

    static_assert(std::future_errc::broken_promise != static_cast<std::future_errc>(0), "");
    static_assert(std::future_errc::future_already_retrieved != static_cast<std::future_errc>(0), "");
    static_assert(std::future_errc::promise_already_satisfied != static_cast<std::future_errc>(0), "");
    static_assert(std::future_errc::no_state != static_cast<std::future_errc>(0), "");
}
