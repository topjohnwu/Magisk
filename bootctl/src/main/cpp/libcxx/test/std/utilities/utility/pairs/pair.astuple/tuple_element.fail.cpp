//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <utility>

// template <class T1, class T2> struct pair

// tuple_element<I, pair<T1, T2> >::type

#include <utility>

int main()
{
    typedef std::pair<int, short> T;
    std::tuple_element<2, T>::type foo; // expected-error@utility:* {{Index out of bounds in std::tuple_element<std::pair<T1, T2>>}}
}
