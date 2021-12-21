//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <list>

// iterator       begin();
// iterator       end();
// const_iterator begin()  const;
// const_iterator end()    const;
// const_iterator cbegin() const;
// const_iterator cend()   const;

#include <list>
#include <cassert>
#include <iterator>

#include "test_macros.h"
#include "min_allocator.h"

struct A
{
    int first;
    int second;
};

int main()
{
    {
        typedef int T;
        typedef std::list<T> C;
        C c;
        C::iterator i = c.begin();
        C::iterator j = c.end();
        assert(std::distance(i, j) == 0);
        assert(i == j);
    }
    {
        typedef int T;
        typedef std::list<T> C;
        const C c;
        C::const_iterator i = c.begin();
        C::const_iterator j = c.end();
        assert(std::distance(i, j) == 0);
        assert(i == j);
    }
    {
        typedef int T;
        typedef std::list<T> C;
        C c;
        C::const_iterator i = c.cbegin();
        C::const_iterator j = c.cend();
        assert(std::distance(i, j) == 0);
        assert(i == j);
        assert(i == c.end());
    }
    {
        typedef int T;
        typedef std::list<T> C;
        const T t[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        C c(std::begin(t), std::end(t));
        C::iterator i = c.begin();
        assert(*i == 0);
        ++i;
        assert(*i == 1);
        *i = 10;
        assert(*i == 10);
        assert(std::distance(c.begin(), c.end()) == 10);
    }
    {
        typedef int T;
        typedef std::list<T> C;
        C::iterator i;
        C::const_iterator j;
    }
#if TEST_STD_VER >= 11
    {
        typedef int T;
        typedef std::list<T, min_allocator<T>> C;
        C c;
        C::iterator i = c.begin();
        C::iterator j = c.end();
        assert(std::distance(i, j) == 0);
        assert(i == j);
    }
    {
        typedef int T;
        typedef std::list<T, min_allocator<T>> C;
        const C c;
        C::const_iterator i = c.begin();
        C::const_iterator j = c.end();
        assert(std::distance(i, j) == 0);
        assert(i == j);
    }
    {
        typedef int T;
        typedef std::list<T, min_allocator<T>> C;
        C c;
        C::const_iterator i = c.cbegin();
        C::const_iterator j = c.cend();
        assert(std::distance(i, j) == 0);
        assert(i == j);
        assert(i == c.end());
    }
    {
        typedef int T;
        typedef std::list<T, min_allocator<T>> C;
        const T t[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        C c(std::begin(t), std::end(t));
        C::iterator i = c.begin();
        assert(*i == 0);
        ++i;
        assert(*i == 1);
        *i = 10;
        assert(*i == 10);
        assert(std::distance(c.begin(), c.end()) == 10);
    }
    {
        typedef int T;
        typedef std::list<T, min_allocator<T>> C;
        C::iterator i;
        C::const_iterator j;
    }
    {
        typedef A T;
        typedef std::list<T, min_allocator<T>> C;
        C c = {A{1, 2}};
        C::iterator i = c.begin();
        i->first = 3;
        C::const_iterator j = i;
        assert(j->first == 3);
    }
#endif
#if TEST_STD_VER > 11
    {
        std::list<int>::iterator ii1{}, ii2{};
        std::list<int>::iterator ii4 = ii1;
        std::list<int>::const_iterator cii{};
        assert ( ii1 == ii2 );
        assert ( ii1 == ii4 );

        assert (!(ii1 != ii2 ));

        assert ( (ii1 == cii ));
        assert ( (cii == ii1 ));
        assert (!(ii1 != cii ));
        assert (!(cii != ii1 ));
    }
#endif

}
