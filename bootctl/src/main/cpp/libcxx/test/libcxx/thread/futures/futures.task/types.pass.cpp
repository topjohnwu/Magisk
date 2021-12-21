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

// <future>

// template<class R, class... ArgTypes>
//     class packaged_task<R(ArgTypes...)>
// {
// public:
//     typedef R result_type; // extension

// This is a libc++ extension.

#include <future>
#include <type_traits>

struct A {};

int main()
{
    static_assert((std::is_same<std::packaged_task<A(int, char)>::result_type, A>::value), "");
}
