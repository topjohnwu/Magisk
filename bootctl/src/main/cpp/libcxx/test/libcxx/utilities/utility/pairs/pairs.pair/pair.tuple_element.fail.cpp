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
    {
    typedef std::pair<int, double> P;
    std::tuple_element<2, P>::type foo; // expected-note {{requested here}}
        // expected-error-re@utility:* {{static_assert failed{{( due to requirement '2U[L]{0,2} < 2')?}} "Index out of bounds in std::tuple_element<std::pair<T1, T2>>"}}
    }
}
