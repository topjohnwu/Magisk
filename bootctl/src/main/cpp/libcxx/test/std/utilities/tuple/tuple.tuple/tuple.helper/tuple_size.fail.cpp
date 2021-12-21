//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <tuple>

// template <class... Types>
//   class tuple_size<tuple<Types...>>
//     : public integral_constant<size_t, sizeof...(Types)> { };

// Expect failures with a reference type, pointer type, and a non-tuple type.

#include <tuple>

int main()
{
    (void)std::tuple_size<std::tuple<> &>::value; // expected-error {{implicit instantiation of undefined template}}
    (void)std::tuple_size<int>::value; // expected-error {{implicit instantiation of undefined template}}
    (void)std::tuple_size<std::tuple<>*>::value; // expected-error {{implicit instantiation of undefined template}}
}
