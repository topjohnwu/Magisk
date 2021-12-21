//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//

// <iterator>

// __libcpp_is_trivial_iterator<Tp>

// __libcpp_is_trivial_iterator determines if an iterator is a "trivial" one,
// that can be used w/o worrying about its operations throwing exceptions.
// Pointers are trivial iterators. Libc++ has three "iterator wrappers":
// reverse_iterator, move_iterator, and __wrap_iter. If the underlying iterator
// is trivial, then those are as well.
//

#include <iterator>
#include <cassert>
#include <string>
#include <vector>
#include <initializer_list>

#include "test_macros.h"
#include "test_iterators.h"

#if TEST_STD_VER >= 11
#define DELETE_FUNCTION = delete
#else
#define DELETE_FUNCTION
#endif

class T;  // incomplete

class my_input_iterator_tag : public std::input_iterator_tag {};

template <class It>
class my_input_iterator
{
    It it_;

    template <class U> friend class my_input_iterator;
public:
    typedef          my_input_iterator_tag                     iterator_category;
    typedef typename std::iterator_traits<It>::value_type      value_type;
    typedef typename std::iterator_traits<It>::difference_type difference_type;
    typedef It                                                 pointer;
    typedef typename std::iterator_traits<It>::reference       reference;

    It base() const {return it_;}

    my_input_iterator() : it_() {}
    explicit my_input_iterator(It it) : it_(it) {}
    template <class U>
        my_input_iterator(const my_input_iterator<U>& u) :it_(u.it_) {}

    reference operator*() const {return *it_;}
    pointer operator->() const {return it_;}

    my_input_iterator& operator++() {++it_; return *this;}
    my_input_iterator operator++(int)
        {my_input_iterator tmp(*this); ++(*this); return tmp;}

    friend bool operator==(const my_input_iterator& x, const my_input_iterator& y)
        {return x.it_ == y.it_;}
    friend bool operator!=(const my_input_iterator& x, const my_input_iterator& y)
        {return !(x == y);}

    template <class T>
    void operator,(T const &) DELETE_FUNCTION;
};

template <class T, class U>
inline
bool
operator==(const my_input_iterator<T>& x, const my_input_iterator<U>& y)
{
    return x.base() == y.base();
}

template <class T, class U>
inline
bool
operator!=(const my_input_iterator<T>& x, const my_input_iterator<U>& y)
{
    return !(x == y);
}


int main()
{
//  basic tests
    static_assert(( std::__libcpp_is_trivial_iterator<char *>::value), "");
    static_assert(( std::__libcpp_is_trivial_iterator<const char *>::value), "");
    static_assert(( std::__libcpp_is_trivial_iterator<int *>::value), "");
    static_assert(( std::__libcpp_is_trivial_iterator<T *>::value), "");

    static_assert(( std::__libcpp_is_trivial_iterator<std::move_iterator<char *> >      ::value), "");
    static_assert(( std::__libcpp_is_trivial_iterator<std::move_iterator<const char *> >::value), "");
    static_assert(( std::__libcpp_is_trivial_iterator<std::move_iterator<int *> >       ::value), "");
    static_assert(( std::__libcpp_is_trivial_iterator<std::move_iterator<T *> >         ::value), "");

    static_assert(( std::__libcpp_is_trivial_iterator<std::reverse_iterator<char *> >      ::value), "");
    static_assert(( std::__libcpp_is_trivial_iterator<std::reverse_iterator<const char *> >::value), "");
    static_assert(( std::__libcpp_is_trivial_iterator<std::reverse_iterator<int *> >       ::value), "");
    static_assert(( std::__libcpp_is_trivial_iterator<std::reverse_iterator<T *> >         ::value), "");

    static_assert(( std::__libcpp_is_trivial_iterator<std::__wrap_iter<char *> >      ::value), "");
    static_assert(( std::__libcpp_is_trivial_iterator<std::__wrap_iter<const char *> >::value), "");
    static_assert(( std::__libcpp_is_trivial_iterator<std::__wrap_iter<int *> >       ::value), "");
    static_assert(( std::__libcpp_is_trivial_iterator<std::__wrap_iter<T *> >         ::value), "");

    static_assert(( std::__libcpp_is_trivial_iterator<std::reverse_iterator<std::__wrap_iter<char *> > > ::value), "");

//  iterators in the libc++ test suite
    static_assert((!std::__libcpp_is_trivial_iterator<output_iterator       <char *> >::value), "");
    static_assert((!std::__libcpp_is_trivial_iterator<input_iterator        <char *> >::value), "");
    static_assert((!std::__libcpp_is_trivial_iterator<forward_iterator      <char *> >::value), "");
    static_assert((!std::__libcpp_is_trivial_iterator<bidirectional_iterator<char *> >::value), "");
    static_assert((!std::__libcpp_is_trivial_iterator<random_access_iterator<char *> >::value), "");
    static_assert((!std::__libcpp_is_trivial_iterator<ThrowingIterator      <char *> >::value), "");
    static_assert((!std::__libcpp_is_trivial_iterator<NonThrowingIterator   <char *> >::value), "");


//  Iterator classification
    static_assert(( std::__is_input_iterator        <char *>::value), "" );
    static_assert(( std::__is_forward_iterator      <char *>::value), "" );
    static_assert(( std::__is_bidirectional_iterator<char *>::value), "" );
    static_assert(( std::__is_random_access_iterator<char *>::value), "" );
    static_assert((!std::__is_exactly_input_iterator<char *>::value), "" );

    static_assert(( std::__is_input_iterator        <input_iterator<char *> >::value), "" );
    static_assert((!std::__is_forward_iterator      <input_iterator<char *> >::value), "" );
    static_assert((!std::__is_bidirectional_iterator<input_iterator<char *> >::value), "" );
    static_assert((!std::__is_random_access_iterator<input_iterator<char *> >::value), "" );
    static_assert(( std::__is_exactly_input_iterator<input_iterator<char *> >::value), "" );

    static_assert(( std::__is_input_iterator        <forward_iterator<char *> >::value), "" );
    static_assert(( std::__is_forward_iterator      <forward_iterator<char *> >::value), "" );
    static_assert((!std::__is_bidirectional_iterator<forward_iterator<char *> >::value), "" );
    static_assert((!std::__is_random_access_iterator<forward_iterator<char *> >::value), "" );
    static_assert((!std::__is_exactly_input_iterator<forward_iterator<char *> >::value), "" );

    static_assert(( std::__is_input_iterator        <bidirectional_iterator<char *> >::value), "" );
    static_assert(( std::__is_forward_iterator      <bidirectional_iterator<char *> >::value), "" );
    static_assert(( std::__is_bidirectional_iterator<bidirectional_iterator<char *> >::value), "" );
    static_assert((!std::__is_random_access_iterator<bidirectional_iterator<char *> >::value), "" );
    static_assert((!std::__is_exactly_input_iterator<bidirectional_iterator<char *> >::value), "" );

    static_assert(( std::__is_input_iterator        <random_access_iterator<char *> >::value), "" );
    static_assert(( std::__is_forward_iterator      <random_access_iterator<char *> >::value), "" );
    static_assert(( std::__is_bidirectional_iterator<random_access_iterator<char *> >::value), "" );
    static_assert(( std::__is_random_access_iterator<random_access_iterator<char *> >::value), "" );
    static_assert((!std::__is_exactly_input_iterator<random_access_iterator<char *> >::value), "" );

    static_assert(( std::__is_input_iterator        <my_input_iterator<char *> >::value), "" );
    static_assert((!std::__is_forward_iterator      <my_input_iterator<char *> >::value), "" );
    static_assert((!std::__is_bidirectional_iterator<my_input_iterator<char *> >::value), "" );
    static_assert((!std::__is_random_access_iterator<my_input_iterator<char *> >::value), "" );
    static_assert(( std::__is_exactly_input_iterator<my_input_iterator<char *> >::value), "" );

//
//  iterators from libc++'s containers
//

//  string
    static_assert(( std::__libcpp_is_trivial_iterator<std::vector<char>::iterator>              ::value), "");
    static_assert(( std::__libcpp_is_trivial_iterator<std::vector<char>::const_iterator>        ::value), "");
    static_assert(( std::__libcpp_is_trivial_iterator<std::vector<char>::reverse_iterator>      ::value), "");
    static_assert(( std::__libcpp_is_trivial_iterator<std::vector<char>::const_reverse_iterator>::value), "");

//  vector
    static_assert(( std::__libcpp_is_trivial_iterator<std::basic_string<char>::iterator>              ::value), "");
    static_assert(( std::__libcpp_is_trivial_iterator<std::basic_string<char>::const_iterator>        ::value), "");
    static_assert(( std::__libcpp_is_trivial_iterator<std::basic_string<char>::reverse_iterator>      ::value), "");
    static_assert(( std::__libcpp_is_trivial_iterator<std::basic_string<char>::const_reverse_iterator>::value), "");

#if TEST_STD_VER >= 11
//  Initializer list  (which has no reverse iterators)
    static_assert(( std::__libcpp_is_trivial_iterator<std::initializer_list<char>::iterator>              ::value), "");
    static_assert(( std::__libcpp_is_trivial_iterator<std::initializer_list<char>::const_iterator>        ::value), "");
#endif

}
