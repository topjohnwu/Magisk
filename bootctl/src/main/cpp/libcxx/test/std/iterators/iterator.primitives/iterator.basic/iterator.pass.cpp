//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <iterator>

// template<class Category, class T, class Distance = ptrdiff_t,
//          class Pointer = T*, class Reference = T&>
// struct iterator
// {
//   typedef T         value_type;
//   typedef Distance  difference_type;
//   typedef Pointer   pointer;
//   typedef Reference reference;
//   typedef Category  iterator_category;
// };

#include <iterator>
#include <type_traits>

struct A {};

template <class T>
void
test2()
{
    typedef std::iterator<std::forward_iterator_tag, T> It;
    static_assert((std::is_same<typename It::value_type, T>::value), "");
    static_assert((std::is_same<typename It::difference_type, std::ptrdiff_t>::value), "");
    static_assert((std::is_same<typename It::pointer, T*>::value), "");
    static_assert((std::is_same<typename It::reference, T&>::value), "");
    static_assert((std::is_same<typename It::iterator_category, std::forward_iterator_tag>::value), "");
}

template <class T>
void
test3()
{
    typedef std::iterator<std::bidirectional_iterator_tag, T, short> It;
    static_assert((std::is_same<typename It::value_type, T>::value), "");
    static_assert((std::is_same<typename It::difference_type, short>::value), "");
    static_assert((std::is_same<typename It::pointer, T*>::value), "");
    static_assert((std::is_same<typename It::reference, T&>::value), "");
    static_assert((std::is_same<typename It::iterator_category, std::bidirectional_iterator_tag>::value), "");
}

template <class T>
void
test4()
{
    typedef std::iterator<std::random_access_iterator_tag, T, int, const T*> It;
    static_assert((std::is_same<typename It::value_type, T>::value), "");
    static_assert((std::is_same<typename It::difference_type, int>::value), "");
    static_assert((std::is_same<typename It::pointer, const T*>::value), "");
    static_assert((std::is_same<typename It::reference, T&>::value), "");
    static_assert((std::is_same<typename It::iterator_category, std::random_access_iterator_tag>::value), "");
}

template <class T>
void
test5()
{
    typedef std::iterator<std::input_iterator_tag, T, long, const T*, const T&> It;
    static_assert((std::is_same<typename It::value_type, T>::value), "");
    static_assert((std::is_same<typename It::difference_type, long>::value), "");
    static_assert((std::is_same<typename It::pointer, const T*>::value), "");
    static_assert((std::is_same<typename It::reference, const T&>::value), "");
    static_assert((std::is_same<typename It::iterator_category, std::input_iterator_tag>::value), "");
}

int main()
{
    test2<A>();
    test3<A>();
    test4<A>();
    test5<A>();
}
