//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <iterator>

// template<class T>
// struct iterator_traits<const T*>
// {
//   typedef ptrdiff_t                  difference_type;
//   typedef T                          value_type;
//   typedef const T*                   pointer;
//   typedef const T&                   reference;
//   typedef random_access_iterator_tag iterator_category;
// };

#include <iterator>
#include <type_traits>

struct A {};

int main()
{
    typedef std::iterator_traits<const A*> It;
    static_assert((std::is_same<It::difference_type, std::ptrdiff_t>::value), "");
    static_assert((std::is_same<It::value_type, A>::value), "");
    static_assert((std::is_same<It::pointer, const A*>::value), "");
    static_assert((std::is_same<It::reference, const A&>::value), "");
    static_assert((std::is_same<It::iterator_category, std::random_access_iterator_tag>::value), "");
}
