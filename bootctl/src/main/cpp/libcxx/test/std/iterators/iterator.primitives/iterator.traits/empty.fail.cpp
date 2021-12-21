//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <iterator>

// struct iterator_traits
// {
// };

#include <iterator>
#include "test_macros.h"

struct A {};
struct NotAnIteratorEmpty {};

struct NotAnIteratorNoDifference
{
//     typedef int                       difference_type;
    typedef A                         value_type;
    typedef A*                        pointer;
    typedef A&                        reference;
    typedef std::forward_iterator_tag iterator_category;
};

struct NotAnIteratorNoValue
{
    typedef int                       difference_type;
//     typedef A                         value_type;
    typedef A*                        pointer;
    typedef A&                        reference;
    typedef std::forward_iterator_tag iterator_category;
};

struct NotAnIteratorNoPointer
{
    typedef int                       difference_type;
    typedef A                         value_type;
//     typedef A*                        pointer;
    typedef A&                        reference;
    typedef std::forward_iterator_tag iterator_category;
};

struct NotAnIteratorNoReference
{
    typedef int                       difference_type;
    typedef A                         value_type;
    typedef A*                        pointer;
//    typedef A&                        reference;
    typedef std::forward_iterator_tag iterator_category;
};

struct NotAnIteratorNoCategory
{
    typedef int                       difference_type;
    typedef A                         value_type;
    typedef A*                        pointer;
    typedef A&                        reference;
//     typedef std::forward_iterator_tag iterator_category;
};

int main()
{
    {
    typedef std::iterator_traits<NotAnIteratorEmpty> T;
    typedef T::difference_type   DT; // expected-error-re {{no type named 'difference_type' in 'std::__1::iterator_traits<{{.+}}>}}
    typedef T::value_type        VT; // expected-error-re {{no type named 'value_type' in 'std::__1::iterator_traits<{{.+}}>}}
    typedef T::pointer           PT; // expected-error-re {{no type named 'pointer' in 'std::__1::iterator_traits<{{.+}}>}}
    typedef T::reference         RT; // expected-error-re {{no type named 'reference' in 'std::__1::iterator_traits<{{.+}}>}}
    typedef T::iterator_category CT; // expected-error-re {{no type named 'iterator_category' in 'std::__1::iterator_traits<{{.+}}>}}
    }

    {
    typedef std::iterator_traits<NotAnIteratorNoDifference> T;
    typedef T::difference_type   DT; // expected-error-re {{no type named 'difference_type' in 'std::__1::iterator_traits<{{.+}}>}}
    typedef T::value_type        VT; // expected-error-re {{no type named 'value_type' in 'std::__1::iterator_traits<{{.+}}>}}
    typedef T::pointer           PT; // expected-error-re {{no type named 'pointer' in 'std::__1::iterator_traits<{{.+}}>}}
    typedef T::reference         RT; // expected-error-re {{no type named 'reference' in 'std::__1::iterator_traits<{{.+}}>}}
    typedef T::iterator_category CT; // expected-error-re {{no type named 'iterator_category' in 'std::__1::iterator_traits<{{.+}}>}}
    }

    {
    typedef std::iterator_traits<NotAnIteratorNoValue> T;
    typedef T::difference_type   DT; // expected-error-re {{no type named 'difference_type' in 'std::__1::iterator_traits<{{.+}}>}}
    typedef T::value_type        VT; // expected-error-re {{no type named 'value_type' in 'std::__1::iterator_traits<{{.+}}>}}
    typedef T::pointer           PT; // expected-error-re {{no type named 'pointer' in 'std::__1::iterator_traits<{{.+}}>}}
    typedef T::reference         RT; // expected-error-re {{no type named 'reference' in 'std::__1::iterator_traits<{{.+}}>}}
    typedef T::iterator_category CT; // expected-error-re {{no type named 'iterator_category' in 'std::__1::iterator_traits<{{.+}}>}}
    }

    {
    typedef std::iterator_traits<NotAnIteratorNoPointer> T;
    typedef T::difference_type   DT; // expected-error-re {{no type named 'difference_type' in 'std::__1::iterator_traits<{{.+}}>}}
    typedef T::value_type        VT; // expected-error-re {{no type named 'value_type' in 'std::__1::iterator_traits<{{.+}}>}}
    typedef T::pointer           PT; // expected-error-re {{no type named 'pointer' in 'std::__1::iterator_traits<{{.+}}>}}
    typedef T::reference         RT; // expected-error-re {{no type named 'reference' in 'std::__1::iterator_traits<{{.+}}>}}
    typedef T::iterator_category CT; // expected-error-re {{no type named 'iterator_category' in 'std::__1::iterator_traits<{{.+}}>}}
    }

    {
    typedef std::iterator_traits<NotAnIteratorNoReference> T;
    typedef T::difference_type   DT; // expected-error-re {{no type named 'difference_type' in 'std::__1::iterator_traits<{{.+}}>}}
    typedef T::value_type        VT; // expected-error-re {{no type named 'value_type' in 'std::__1::iterator_traits<{{.+}}>}}
    typedef T::pointer           PT; // expected-error-re {{no type named 'pointer' in 'std::__1::iterator_traits<{{.+}}>}}
    typedef T::reference         RT; // expected-error-re {{no type named 'reference' in 'std::__1::iterator_traits<{{.+}}>}}
    typedef T::iterator_category CT; // expected-error-re {{no type named 'iterator_category' in 'std::__1::iterator_traits<{{.+}}>}}
    }

    {
    typedef std::iterator_traits<NotAnIteratorNoCategory> T;
    typedef T::difference_type   DT; // expected-error-re {{no type named 'difference_type' in 'std::__1::iterator_traits<{{.+}}>}}
    typedef T::value_type        VT; // expected-error-re {{no type named 'value_type' in 'std::__1::iterator_traits<{{.+}}>}}
    typedef T::pointer           PT; // expected-error-re {{no type named 'pointer' in 'std::__1::iterator_traits<{{.+}}>}}
    typedef T::reference         RT; // expected-error-re {{no type named 'reference' in 'std::__1::iterator_traits<{{.+}}>}}
    typedef T::iterator_category CT; // expected-error-re {{no type named 'iterator_category' in 'std::__1::iterator_traits<{{.+}}>}}
    }
}
