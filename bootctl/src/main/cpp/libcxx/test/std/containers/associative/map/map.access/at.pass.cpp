//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <map>

// class map

//       mapped_type& at(const key_type& k);
// const mapped_type& at(const key_type& k) const;

#include <cassert>
#include <map>
#include <stdexcept>

#include "min_allocator.h"
#include "test_macros.h"

int main()
{
    {
        typedef std::pair<const int, double> V;
        V ar[] =
        {
            V(1, 1.5),
            V(2, 2.5),
            V(3, 3.5),
            V(4, 4.5),
            V(5, 5.5),
            V(7, 7.5),
            V(8, 8.5),
        };
        std::map<int, double> m(ar, ar+sizeof(ar)/sizeof(ar[0]));
        assert(m.size() == 7);
        assert(m.at(1) == 1.5);
        m.at(1) = -1.5;
        assert(m.at(1) == -1.5);
        assert(m.at(2) == 2.5);
        assert(m.at(3) == 3.5);
        assert(m.at(4) == 4.5);
        assert(m.at(5) == 5.5);
#ifndef TEST_HAS_NO_EXCEPTIONS
        try
        {
            TEST_IGNORE_NODISCARD m.at(6);
            assert(false);
        }
        catch (std::out_of_range&)
        {
        }
#endif
        assert(m.at(7) == 7.5);
        assert(m.at(8) == 8.5);
        assert(m.size() == 7);
    }
    {
        typedef std::pair<const int, double> V;
        V ar[] =
        {
            V(1, 1.5),
            V(2, 2.5),
            V(3, 3.5),
            V(4, 4.5),
            V(5, 5.5),
            V(7, 7.5),
            V(8, 8.5),
        };
        const std::map<int, double> m(ar, ar+sizeof(ar)/sizeof(ar[0]));
        assert(m.size() == 7);
        assert(m.at(1) == 1.5);
        assert(m.at(2) == 2.5);
        assert(m.at(3) == 3.5);
        assert(m.at(4) == 4.5);
        assert(m.at(5) == 5.5);
#ifndef TEST_HAS_NO_EXCEPTIONS
        try
        {
            TEST_IGNORE_NODISCARD m.at(6);
            assert(false);
        }
        catch (std::out_of_range&)
        {
        }
#endif
        assert(m.at(7) == 7.5);
        assert(m.at(8) == 8.5);
        assert(m.size() == 7);
    }
#if TEST_STD_VER >= 11
    {
        typedef std::pair<const int, double> V;
        V ar[] =
        {
            V(1, 1.5),
            V(2, 2.5),
            V(3, 3.5),
            V(4, 4.5),
            V(5, 5.5),
            V(7, 7.5),
            V(8, 8.5),
        };
        std::map<int, double, std::less<int>, min_allocator<V>> m(ar, ar+sizeof(ar)/sizeof(ar[0]));
        assert(m.size() == 7);
        assert(m.at(1) == 1.5);
        m.at(1) = -1.5;
        assert(m.at(1) == -1.5);
        assert(m.at(2) == 2.5);
        assert(m.at(3) == 3.5);
        assert(m.at(4) == 4.5);
        assert(m.at(5) == 5.5);
#ifndef TEST_HAS_NO_EXCEPTIONS
        try
        {
            TEST_IGNORE_NODISCARD m.at(6);
            assert(false);
        }
        catch (std::out_of_range&)
        {
        }
#endif
        assert(m.at(7) == 7.5);
        assert(m.at(8) == 8.5);
        assert(m.size() == 7);
    }
    {
        typedef std::pair<const int, double> V;
        V ar[] =
        {
            V(1, 1.5),
            V(2, 2.5),
            V(3, 3.5),
            V(4, 4.5),
            V(5, 5.5),
            V(7, 7.5),
            V(8, 8.5),
        };
        const std::map<int, double, std::less<int>, min_allocator<V>> m(ar, ar+sizeof(ar)/sizeof(ar[0]));
        assert(m.size() == 7);
        assert(m.at(1) == 1.5);
        assert(m.at(2) == 2.5);
        assert(m.at(3) == 3.5);
        assert(m.at(4) == 4.5);
        assert(m.at(5) == 5.5);
#ifndef TEST_HAS_NO_EXCEPTIONS
        try
        {
            TEST_IGNORE_NODISCARD m.at(6);
            assert(false);
        }
        catch (std::out_of_range&)
        {
        }
#endif
        assert(m.at(7) == 7.5);
        assert(m.at(8) == 8.5);
        assert(m.size() == 7);
    }
#endif
}
