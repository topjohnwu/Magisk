//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <iterator>

// template<class NotAnIterator>
// struct iterator_traits
// {
// };

#include <iterator>

struct not_an_iterator
{
};

template <class T>
struct has_value_type
{
private:
    struct two {char lx; char lxx;};
    template <class U> static two test(...);
    template <class U> static char test(typename U::value_type* = 0);
public:
    static const bool value = sizeof(test<T>(0)) == 1;
};

int main()
{
    typedef std::iterator_traits<not_an_iterator> It;
    static_assert(!(has_value_type<It>::value), "");
}
