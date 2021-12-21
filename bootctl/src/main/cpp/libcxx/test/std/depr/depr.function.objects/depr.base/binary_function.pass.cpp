//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>
// REQUIRES: c++98 || c++03 || c++11 || c++14
// binary_function was removed in C++17

// template <class Arg1, class Arg2, class Result>
// struct binary_function
// {
//     typedef Arg1   first_argument_type;
//     typedef Arg2   second_argument_type;
//     typedef Result result_type;
// };

#include <functional>
#include <type_traits>

int main()
{
    static_assert((std::is_same<std::binary_function<int, unsigned, char>::first_argument_type, int>::value), "");
    static_assert((std::is_same<std::binary_function<int, unsigned, char>::second_argument_type, unsigned>::value), "");
    static_assert((std::is_same<std::binary_function<int, unsigned, char>::result_type, char>::value), "");
}
