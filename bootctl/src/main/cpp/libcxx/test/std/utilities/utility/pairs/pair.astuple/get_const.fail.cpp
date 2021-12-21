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

// template<size_t I, class T1, class T2>
//     const typename tuple_element<I, std::pair<T1, T2> >::type&
//     get(const pair<T1, T2>&);

#include <utility>
#include <cassert>

int main()
{
    {
        typedef std::pair<int, short> P;
        const P p(3, 4);
        assert(std::get<0>(p) == 3);
        assert(std::get<1>(p) == 4);
        std::get<0>(p) = 5;
    }
}
