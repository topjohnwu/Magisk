//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <memory>

// template <class Ptr>
// struct pointer_traits
// {
//     static pointer pointer_to(<details>);
//     ...
// };

#include <memory>
#include <cassert>

template <class T>
struct A
{
private:
    struct nat {};
public:
    typedef T element_type;
    element_type* t_;

    A(element_type* t) : t_(t) {}

    static A pointer_to(typename std::conditional<std::is_void<element_type>::value,
                                           nat, element_type>::type& et)
        {return A(&et);}
};

int main()
{
    {
        int i = 0;
        static_assert((std::is_same<A<int>, decltype(std::pointer_traits<A<int> >::pointer_to(i))>::value), "");
        A<int> a = std::pointer_traits<A<int> >::pointer_to(i);
        assert(a.t_ == &i);
    }
    {
        (std::pointer_traits<A<void> >::element_type)0;
    }
}
