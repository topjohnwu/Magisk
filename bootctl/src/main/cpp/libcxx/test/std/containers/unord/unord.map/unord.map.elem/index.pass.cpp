//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <unordered_map>

// template <class Key, class T, class Hash = hash<Key>, class Pred = equal_to<Key>,
//           class Alloc = allocator<pair<const Key, T>>>
// class unordered_map

// mapped_type& operator[](const key_type& k);
// mapped_type& operator[](key_type&& k);

#include <unordered_map>
#include <string>
#include <cassert>

#include "test_macros.h"
#include "MoveOnly.h"
#include "min_allocator.h"
#include "count_new.hpp"

#if TEST_STD_VER >= 11
#include "container_test_types.h"
#endif

int main()
{
    {
        typedef std::unordered_map<int, std::string> C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        C c(a, a + sizeof(a)/sizeof(a[0]));
        assert(c.size() == 4);
        c[1] = "ONE";
        assert(c.at(1) == "ONE");
        c[11] = "eleven";
        assert(c.size() == 5);
        assert(c.at(11) == "eleven");
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_map<MoveOnly, std::string> C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        C c(a, a + sizeof(a)/sizeof(a[0]));
        assert(c.size() == 4);
        c[1] = "ONE";
        assert(c.at(1) == "ONE");
        c[11] = "eleven";
        assert(c.size() == 5);
        assert(c.at(11) == "eleven");
    }
    {
        typedef std::unordered_map<int, std::string, std::hash<int>, std::equal_to<int>,
                            min_allocator<std::pair<const int, std::string>>> C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        C c(a, a + sizeof(a)/sizeof(a[0]));
        assert(c.size() == 4);
        c[1] = "ONE";
        assert(c.at(1) == "ONE");
        c[11] = "eleven";
        assert(c.size() == 5);
        assert(c.at(11) == "eleven");
    }

    {
        typedef std::unordered_map<MoveOnly, std::string, std::hash<MoveOnly>, std::equal_to<MoveOnly>,
                            min_allocator<std::pair<const MoveOnly, std::string>>> C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        C c(a, a + sizeof(a)/sizeof(a[0]));
        assert(c.size() == 4);
        c[1] = "ONE";
        assert(c.at(1) == "ONE");
        c[11] = "eleven";
        assert(c.size() == 5);
        assert(c.at(11) == "eleven");
    }
    {
        using Container = TCT::unordered_map<>;
        using Key = Container::key_type;
        using MappedType = Container::mapped_type;
        ConstructController* cc = getConstructController();
        cc->reset();
        {
            Container c;
            const Key k(1);
            cc->expect<std::piecewise_construct_t const&, std::tuple<Key const&>&&, std::tuple<>&&>();
            MappedType& mref = c[k];
            assert(!cc->unchecked());
            {
                DisableAllocationGuard g;
                MappedType& mref2 = c[k];
                assert(&mref == &mref2);
            }
        }
        {
            Container c;
            Key k(1);
            cc->expect<std::piecewise_construct_t const&, std::tuple<Key const&>&&, std::tuple<>&&>();
            MappedType& mref = c[k];
            assert(!cc->unchecked());
            {
                DisableAllocationGuard g;
                MappedType& mref2 = c[k];
                assert(&mref == &mref2);
            }
        }
        {
            Container c;
            Key k(1);
            cc->expect<std::piecewise_construct_t const&, std::tuple<Key &&>&&, std::tuple<>&&>();
            MappedType& mref = c[std::move(k)];
            assert(!cc->unchecked());
            {
                Key k2(1);
                DisableAllocationGuard g;
                MappedType& mref2 = c[std::move(k2)];
                assert(&mref == &mref2);
            }
        }
    }
#endif
}
