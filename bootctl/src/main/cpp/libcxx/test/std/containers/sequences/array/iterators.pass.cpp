//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <array>

// iterator, const_iterator

#include <array>
#include <iterator>
#include <cassert>

#include "test_macros.h"

// std::array is explicitly allowed to be initialized with A a = { init-list };.
// Disable the missing braces warning for this reason.
#include "disable_missing_braces_warning.h"

int main()
{
    {
    typedef std::array<int, 5> C;
    C c;
    C::iterator i;
    i = c.begin();
    C::const_iterator j;
    j = c.cbegin();
    assert(i == j);
    }
    {
    typedef std::array<int, 0> C;
    C c;
    C::iterator i;
    i = c.begin();
    C::const_iterator j;
    j = c.cbegin();
    assert(i == j);
    }

#if TEST_STD_VER > 11
    { // N3644 testing
        {
        typedef std::array<int, 5> C;
        C::iterator ii1{}, ii2{};
        C::iterator ii4 = ii1;
        C::const_iterator cii{};
        assert ( ii1 == ii2 );
        assert ( ii1 == ii4 );
        assert ( ii1 == cii );

        assert ( !(ii1 != ii2 ));
        assert ( !(ii1 != cii ));

        C c;
        assert ( c.begin()   == std::begin(c));
        assert ( c.cbegin()  == std::cbegin(c));
        assert ( c.rbegin()  == std::rbegin(c));
        assert ( c.crbegin() == std::crbegin(c));
        assert ( c.end()     == std::end(c));
        assert ( c.cend()    == std::cend(c));
        assert ( c.rend()    == std::rend(c));
        assert ( c.crend()   == std::crend(c));

        assert ( std::begin(c)   != std::end(c));
        assert ( std::rbegin(c)  != std::rend(c));
        assert ( std::cbegin(c)  != std::cend(c));
        assert ( std::crbegin(c) != std::crend(c));
        }
        {
        typedef std::array<int, 0> C;
        C::iterator ii1{}, ii2{};
        C::iterator ii4 = ii1;
        C::const_iterator cii{};
        assert ( ii1 == ii2 );
        assert ( ii1 == ii4 );

        assert (!(ii1 != ii2 ));

        assert ( (ii1 == cii ));
        assert ( (cii == ii1 ));
        assert (!(ii1 != cii ));
        assert (!(cii != ii1 ));
        assert (!(ii1 <  cii ));
        assert (!(cii <  ii1 ));
        assert ( (ii1 <= cii ));
        assert ( (cii <= ii1 ));
        assert (!(ii1 >  cii ));
        assert (!(cii >  ii1 ));
        assert ( (ii1 >= cii ));
        assert ( (cii >= ii1 ));
        assert (cii - ii1 == 0);
        assert (ii1 - cii == 0);

        C c;
        assert ( c.begin()   == std::begin(c));
        assert ( c.cbegin()  == std::cbegin(c));
        assert ( c.rbegin()  == std::rbegin(c));
        assert ( c.crbegin() == std::crbegin(c));
        assert ( c.end()     == std::end(c));
        assert ( c.cend()    == std::cend(c));
        assert ( c.rend()    == std::rend(c));
        assert ( c.crend()   == std::crend(c));

        assert ( std::begin(c)   == std::end(c));
        assert ( std::rbegin(c)  == std::rend(c));
        assert ( std::cbegin(c)  == std::cend(c));
        assert ( std::crbegin(c) == std::crend(c));
        }
    }
#endif
#if TEST_STD_VER > 14
    {
        typedef std::array<int, 5> C;
        constexpr C c{0,1,2,3,4};

        static_assert ( c.begin()   == std::begin(c), "");
        static_assert ( c.cbegin()  == std::cbegin(c), "");
        static_assert ( c.end()     == std::end(c), "");
        static_assert ( c.cend()    == std::cend(c), "");

        static_assert ( c.rbegin()  == std::rbegin(c), "");
        static_assert ( c.crbegin() == std::crbegin(c), "");
        static_assert ( c.rend()    == std::rend(c), "");
        static_assert ( c.crend()   == std::crend(c), "");

        static_assert ( std::begin(c)   != std::end(c), "");
        static_assert ( std::rbegin(c)  != std::rend(c), "");
        static_assert ( std::cbegin(c)  != std::cend(c), "");
        static_assert ( std::crbegin(c) != std::crend(c), "");

        static_assert ( *c.begin()  == 0, "");
        static_assert ( *c.rbegin()  == 4, "");

        static_assert ( *std::begin(c)   == 0, "" );
        static_assert ( *std::cbegin(c)  == 0, "" );
        static_assert ( *std::rbegin(c)  == 4, "" );
        static_assert ( *std::crbegin(c) == 4, "" );
    }
#endif
}
