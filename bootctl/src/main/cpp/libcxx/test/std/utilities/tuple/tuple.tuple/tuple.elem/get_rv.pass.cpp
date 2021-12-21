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
//   typename tuple_element<I, tuple<Types...> >::type&&
//   get(tuple<Types...>&& t);

// UNSUPPORTED: c++98, c++03

#include <tuple>
#include <utility>
#include <memory>
#include <cassert>

int main()
{
    {
        typedef std::tuple<std::unique_ptr<int> > T;
        T t(std::unique_ptr<int>(new int(3)));
        std::unique_ptr<int> p = std::get<0>(std::move(t));
        assert(*p == 3);
    }
}
