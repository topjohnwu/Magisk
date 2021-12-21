//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14

// <map>

// class multimap

// template <class C2>
//   void merge(map<key_type, value_type, C2, allocator_type>& source);
// template <class C2>
//   void merge(map<key_type, value_type, C2, allocator_type>&& source);
// template <class C2>
//   void merge(multimap<key_type, value_type, C2, allocator_type>& source);
// template <class C2>
//   void merge(multimap<key_type, value_type, C2, allocator_type>&& source);

#include <map>
#include <cassert>
#include "test_macros.h"
#include "Counter.h"

template <class Map>
bool map_equal(const Map& map, Map other)
{
    return map == other;
}

#ifndef TEST_HAS_NO_EXCEPTIONS
struct throw_comparator
{
    bool& should_throw_;

    throw_comparator(bool& should_throw) : should_throw_(should_throw) {}

    template <class T>
    bool operator()(const T& lhs, const T& rhs) const
    {
        if (should_throw_)
            throw 0;
        return lhs < rhs;
    }
};
#endif

int main()
{
    {
        std::multimap<int, int> src{{1, 0}, {3, 0}, {5, 0}};
        std::multimap<int, int> dst{{2, 0}, {4, 0}, {5, 0}};
        dst.merge(src);
        assert(map_equal(src, {}));
        assert(map_equal(dst, {{1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}, {5, 0}}));
    }

#ifndef TEST_HAS_NO_EXCEPTIONS
    {
        bool do_throw = false;
        typedef std::multimap<Counter<int>, int, throw_comparator> map_type;
        map_type src({{1, 0}, {3, 0}, {5, 0}}, throw_comparator(do_throw));
        map_type dst({{2, 0}, {4, 0}, {5, 0}}, throw_comparator(do_throw));

        assert(Counter_base::gConstructed == 6);

        do_throw = true;
        try
        {
            dst.merge(src);
        }
        catch (int)
        {
            do_throw = false;
        }
        assert(!do_throw);
        assert(map_equal(src, map_type({{1, 0}, {3, 0}, {5, 0}}, throw_comparator(do_throw))));
        assert(map_equal(dst, map_type({{2, 0}, {4, 0}, {5, 0}}, throw_comparator(do_throw))));
    }
#endif
    assert(Counter_base::gConstructed == 0);
    struct comparator
    {
        comparator() = default;

        bool operator()(const Counter<int>& lhs, const Counter<int>& rhs) const
        {
            return lhs < rhs;
        }
    };
    {
        typedef std::multimap<Counter<int>, int, std::less<Counter<int>>> first_map_type;
        typedef std::multimap<Counter<int>, int, comparator> second_map_type;
        typedef std::map<Counter<int>, int, comparator> third_map_type;

        {
            first_map_type first{{1, 0}, {2, 0}, {3, 0}};
            second_map_type second{{2, 0}, {3, 0}, {4, 0}};
            third_map_type third{{1, 0}, {3, 0}};

            assert(Counter_base::gConstructed == 8);

            first.merge(second);
            first.merge(third);

            assert(map_equal(first, {{1, 0}, {1, 0}, {2, 0}, {2, 0}, {3, 0}, {3, 0}, {3, 0}, {4, 0}}));
            assert(map_equal(second, {}));
            assert(map_equal(third, {}));

            assert(Counter_base::gConstructed == 8);
        }
        assert(Counter_base::gConstructed == 0);
        {
            first_map_type first{{1, 0}, {2, 0}, {3, 0}};
            second_map_type second{{2, 0}, {3, 0}, {4, 0}};
            third_map_type third{{1, 0}, {3, 0}};

            assert(Counter_base::gConstructed == 8);

            first.merge(std::move(second));
            first.merge(std::move(third));

            assert(map_equal(first, {{1, 0}, {1, 0}, {2, 0}, {2, 0}, {3, 0}, {3, 0}, {3, 0}, {4, 0}}));
            assert(map_equal(second, {}));
            assert(map_equal(third, {}));

            assert(Counter_base::gConstructed == 8);
        }
        assert(Counter_base::gConstructed == 0);
    }
    assert(Counter_base::gConstructed == 0);
    {
        std::multimap<int, int> first;
        {
            std::multimap<int, int> second;
            first.merge(second);
            first.merge(std::move(second));
        }
        {
            std::multimap<int, int> second;
            first.merge(second);
            first.merge(std::move(second));
        }
    }
}
