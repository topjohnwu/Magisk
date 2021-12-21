//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <optional>
// UNSUPPORTED: c++98, c++03, c++11, c++14
// UNSUPPORTED: clang-5
// UNSUPPORTED: libcpp-no-deduction-guides
// Clang 5 will generate bad implicit deduction guides
//  Specifically, for the copy constructor.


// template<class T>
//   optional(T) -> optional<T>;


#include <optional>
#include <cassert>

struct A {};

int main()
{
//  Test the explicit deduction guides

//  Test the implicit deduction guides
    {
//  optional()
    std::optional opt;   // expected-error-re {{{{declaration of variable 'opt' with deduced type 'std::optional' requires an initializer|no viable constructor or deduction guide for deduction of template arguments of 'optional'}}}}
//  clang-6 gives a bogus error here:
//      declaration of variable 'opt' with deduced type 'std::optional' requires an initializer
//  clang-7 (and later) give a better message:
//      no viable constructor or deduction guide for deduction of template arguments of 'optional'
//  So we check for one or the other.
    }

    {
//  optional(nullopt_t)
    std::optional opt(std::nullopt);   // expected-error-re@optional:* {{static_assert failed{{.*}} "instantiation of optional with nullopt_t is ill-formed"}}
    }
}
