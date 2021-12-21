//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <tuple>

// template <class... Types> class tuple;

// template <size_t I, class... Types>
// class tuple_element<I, tuple<Types...> >
// {
// public:
//     typedef Ti type;
// };

// UNSUPPORTED: c++98, c++03

#include <tuple>
#include <type_traits>

int main()
{
    using T =  std::tuple<int, long, void*>;
    using E1 = typename std::tuple_element<1, T &>::type; // expected-error{{undefined template}}
    using E2 = typename std::tuple_element<3, T>::type;
    using E3 = typename std::tuple_element<4, T const>::type;
        // expected-error@__tuple:* 2 {{static_assert failed}}

}
