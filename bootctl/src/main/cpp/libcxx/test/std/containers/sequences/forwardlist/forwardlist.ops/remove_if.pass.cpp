//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <forward_list>

// template <class Predicate> void remove_if(Predicate pred);

#include <forward_list>
#include <iterator>
#include <cassert>
#include <cstddef>

#include "min_allocator.h"
#include "counting_predicates.hpp"


bool g(int i)
{
    return i < 3;
}

int main()
{
    {
        typedef int T;
        typedef unary_counting_predicate<bool(*)(T), T> Predicate;
        typedef std::forward_list<T> C;
        const T t1[] = {0, 5, 5, 0, 0, 0, 5};
        const T t2[] = {5, 5, 5};
        C c1(std::begin(t1), std::end(t1));
        C c2(std::begin(t2), std::end(t2));
        Predicate cp(g);
        c1.remove_if(std::ref(cp));
        assert(c1 == c2);
        assert(cp.count() == static_cast<std::size_t>(std::distance(std::begin(t1), std::end(t1))));
    }
    {
        typedef int T;
        typedef unary_counting_predicate<bool(*)(T), T> Predicate;
        typedef std::forward_list<T> C;
        const T t1[] = {0, 0, 0, 0};
        C c1(std::begin(t1), std::end(t1));
        C c2;
        Predicate cp(g);
        c1.remove_if(std::ref(cp));
        assert(c1 == c2);
        assert(cp.count() == static_cast<std::size_t>(std::distance(std::begin(t1), std::end(t1))));
    }
    {
        typedef int T;
        typedef unary_counting_predicate<bool(*)(T), T> Predicate;
        typedef std::forward_list<T> C;
        const T t1[] = {5, 5, 5};
        const T t2[] = {5, 5, 5};
        C c1(std::begin(t1), std::end(t1));
        C c2(std::begin(t2), std::end(t2));
        Predicate cp(g);
        c1.remove_if(std::ref(cp));
        assert(c1 == c2);
        assert(cp.count() == static_cast<std::size_t>(std::distance(std::begin(t1), std::end(t1))));
    }
    {
        typedef int T;
        typedef unary_counting_predicate<bool(*)(T), T> Predicate;
        typedef std::forward_list<T> C;
        C c1;
        C c2;
        Predicate cp(g);
        c1.remove_if(std::ref(cp));
        assert(c1 == c2);
        assert(cp.count() == 0);
    }
    {
        typedef int T;
        typedef unary_counting_predicate<bool(*)(T), T> Predicate;
        typedef std::forward_list<T> C;
        const T t1[] = {5, 5, 5, 0};
        const T t2[] = {5, 5, 5};
        C c1(std::begin(t1), std::end(t1));
        C c2(std::begin(t2), std::end(t2));
        Predicate cp(g);
        c1.remove_if(std::ref(cp));
        assert(c1 == c2);
        assert(cp.count() == static_cast<std::size_t>(std::distance(std::begin(t1), std::end(t1))));
    }
#if TEST_STD_VER >= 11
    {
        typedef int T;
        typedef unary_counting_predicate<bool(*)(T), T> Predicate;
        typedef std::forward_list<T, min_allocator<T>> C;
        const T t1[] = {0, 5, 5, 0, 0, 0, 5};
        const T t2[] = {5, 5, 5};
        C c1(std::begin(t1), std::end(t1));
        C c2(std::begin(t2), std::end(t2));
        Predicate cp(g);
        c1.remove_if(std::ref(cp));
        assert(c1 == c2);
        assert(cp.count() == static_cast<std::size_t>(std::distance(std::begin(t1), std::end(t1))));
    }
    {
        typedef int T;
        typedef unary_counting_predicate<bool(*)(T), T> Predicate;
        typedef std::forward_list<T, min_allocator<T>> C;
        const T t1[] = {0, 0, 0, 0};
        C c1(std::begin(t1), std::end(t1));
        C c2;
        Predicate cp(g);
        c1.remove_if(std::ref(cp));
        assert(c1 == c2);
        assert(cp.count() == static_cast<std::size_t>(std::distance(std::begin(t1), std::end(t1))));
    }
    {
        typedef int T;
        typedef unary_counting_predicate<bool(*)(T), T> Predicate;
        typedef std::forward_list<T, min_allocator<T>> C;
        const T t1[] = {5, 5, 5};
        const T t2[] = {5, 5, 5};
        C c1(std::begin(t1), std::end(t1));
        C c2(std::begin(t2), std::end(t2));
        Predicate cp(g);
        c1.remove_if(std::ref(cp));
        assert(c1 == c2);
        assert(cp.count() == static_cast<std::size_t>(std::distance(std::begin(t1), std::end(t1))));
    }
    {
        typedef int T;
        typedef unary_counting_predicate<bool(*)(T), T> Predicate;
        typedef std::forward_list<T, min_allocator<T>> C;
        C c1;
        C c2;
        Predicate cp(g);
        c1.remove_if(std::ref(cp));
        assert(c1 == c2);
        assert(cp.count() == 0);
    }
    {
        typedef int T;
        typedef unary_counting_predicate<bool(*)(T), T> Predicate;
        typedef std::forward_list<T, min_allocator<T>> C;
        const T t1[] = {5, 5, 5, 0};
        const T t2[] = {5, 5, 5};
        C c1(std::begin(t1), std::end(t1));
        C c2(std::begin(t2), std::end(t2));
        Predicate cp(g);
        c1.remove_if(std::ref(cp));
        assert(c1 == c2);
        assert(cp.count() == static_cast<std::size_t>(std::distance(std::begin(t1), std::end(t1))));
    }
#endif
}
