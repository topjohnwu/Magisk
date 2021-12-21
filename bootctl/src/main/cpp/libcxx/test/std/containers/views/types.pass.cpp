// -*- C++ -*-
//===------------------------------ span ---------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===---------------------------------------------------------------------===//
// UNSUPPORTED: c++98, c++03, c++11, c++14, c++17

// <span>

// template<class ElementType, ptrdiff_t Extent = dynamic_extent>
// class span {
// public:
//  // constants and types
//  using element_type           = ElementType;
//  using value_type             = remove_cv_t<ElementType>;
//  using index_type             = ptrdiff_t;
//  using difference_type        = ptrdiff_t;
//  using pointer                = element_type *;
//  using reference              = element_type &;
//  using iterator               = implementation-defined;
//  using const_iterator         = implementation-defined;
//  using reverse_iterator       = std::reverse_iterator<iterator>;
//  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
//
//  static constexpr index_type extent = Extent;
//

#include <span>
#include <cassert>
#include <iterator>
#include <string>

#include "test_macros.h"

template <typename S, typename Iter>
void testIterator()
{
    typedef std::iterator_traits<Iter> ItT;

    ASSERT_SAME_TYPE(typename ItT::iterator_category, std::random_access_iterator_tag);
    ASSERT_SAME_TYPE(typename ItT::value_type,        typename S::value_type);
    ASSERT_SAME_TYPE(typename ItT::reference,         typename S::reference);
    ASSERT_SAME_TYPE(typename ItT::pointer,           typename S::pointer);
    ASSERT_SAME_TYPE(typename ItT::difference_type,   typename S::difference_type);
}

template <typename S, typename Iter>
void testConstIterator()
{
    typedef std::iterator_traits<Iter> ItT;

    ASSERT_SAME_TYPE(typename ItT::iterator_category, std::random_access_iterator_tag);
    ASSERT_SAME_TYPE(typename ItT::value_type,        typename S::value_type);
//  I'd like to say 'const typename S::pointer' here, but that gives me
//      a const pointer to a non-const value, which is not what I want.
    ASSERT_SAME_TYPE(typename ItT::reference,         typename S::element_type const &);
    ASSERT_SAME_TYPE(typename ItT::pointer,           typename S::element_type const *);
    ASSERT_SAME_TYPE(typename ItT::difference_type,   typename S::difference_type);
}

template <typename S, typename ElementType, std::ptrdiff_t Size>
void testSpan()
{
    ASSERT_SAME_TYPE(typename S::element_type,    ElementType);
    ASSERT_SAME_TYPE(typename S::value_type,      std::remove_cv_t<ElementType>);
    ASSERT_SAME_TYPE(typename S::index_type,      std::ptrdiff_t);
    ASSERT_SAME_TYPE(typename S::difference_type, std::ptrdiff_t);
    ASSERT_SAME_TYPE(typename S::pointer,         ElementType *);
    ASSERT_SAME_TYPE(typename S::reference,       ElementType &);

    static_assert(S::extent == Size); // check that it exists

    testIterator<S, typename S::iterator>();
    testIterator<S, typename S::reverse_iterator>();
    testConstIterator<S, typename S::const_iterator>();
    testConstIterator<S, typename S::const_reverse_iterator>();
}


template <typename T>
void test()
{
    testSpan<std::span<               T>,                T, -1>();
    testSpan<std::span<const          T>, const          T, -1>();
    testSpan<std::span<      volatile T>,       volatile T, -1>();
    testSpan<std::span<const volatile T>, const volatile T, -1>();

    testSpan<std::span<               T, 5>,                T, 5>();
    testSpan<std::span<const          T, 5>, const          T, 5>();
    testSpan<std::span<      volatile T, 5>,       volatile T, 5>();
    testSpan<std::span<const volatile T, 5>, const volatile T, 5>();
}

struct A{};

int main ()
{
    test<int>();
    test<long>();
    test<double>();
    test<std::string>();
    test<A>();
}
