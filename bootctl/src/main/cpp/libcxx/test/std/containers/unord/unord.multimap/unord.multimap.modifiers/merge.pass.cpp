//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14

// <unordered_map>

// class unordered_multimap

// template <class H2, class P2>
//   void merge(unordered_map<key_type, value_type, H2, P2, allocator_type>& source);
// template <class H2, class P2>
//   void merge(unordered_map<key_type, value_type, H2, P2, allocator_type>&& source);
// template <class H2, class P2>
//   void merge(unordered_multimap<key_type, value_type, H2, P2, allocator_type>& source);
// template <class H2, class P2>
//   void merge(unordered_multimap<key_type, value_type, H2, P2, allocator_type>&& source);

#include <unordered_map>
#include <cassert>
#include "test_macros.h"
#include "Counter.h"

template <class Map>
bool map_equal(const Map& map, Map other)
{
    return map == other;
}

#ifndef TEST_HAS_NO_EXCEPTIONS
template <class T>
struct throw_hasher
{
    bool& should_throw_;

    throw_hasher(bool& should_throw) : should_throw_(should_throw) {}

    size_t operator()(const T& p) const
    {
        if (should_throw_)
            throw 0;
        return std::hash<T>()(p);
    }
};
#endif

int main()
{
    {
        std::unordered_multimap<int, int> src{{1, 0}, {3, 0}, {5, 0}};
        std::unordered_multimap<int, int> dst{{2, 0}, {4, 0}, {5, 0}};
        dst.merge(src);
        assert(map_equal(src, {}));
        assert(map_equal(dst, {{1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}, {5, 0}}));
    }

#ifndef TEST_HAS_NO_EXCEPTIONS
    {
        bool do_throw = false;
        typedef std::unordered_multimap<Counter<int>, int, throw_hasher<Counter<int>>> map_type;
        map_type src({{1, 0}, {3, 0}, {5, 0}}, 0, throw_hasher<Counter<int>>(do_throw));
        map_type dst({{2, 0}, {4, 0}, {5, 0}}, 0, throw_hasher<Counter<int>>(do_throw));

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
        assert(map_equal(src, map_type({{1, 0}, {3, 0}, {5, 0}}, 0, throw_hasher<Counter<int>>(do_throw))));
        assert(map_equal(dst, map_type({{2, 0}, {4, 0}, {5, 0}}, 0, throw_hasher<Counter<int>>(do_throw))));
    }
#endif
    assert(Counter_base::gConstructed == 0);
    struct equal
    {
        equal() = default;

        bool operator()(const Counter<int>& lhs, const Counter<int>& rhs) const
        {
            return lhs == rhs;
        }
    };
    struct hasher
    {
        hasher() = default;
        size_t operator()(const Counter<int>& p) const
        {
            return std::hash<Counter<int>>()(p);
        }
    };
    {
        typedef std::unordered_multimap<Counter<int>, int, std::hash<Counter<int>>, std::equal_to<Counter<int>>> first_map_type;
        typedef std::unordered_multimap<Counter<int>, int, hasher, equal> second_map_type;
        typedef std::unordered_map<Counter<int>, int, hasher, equal> third_map_type;

        {
            first_map_type first{{1, 0}, {2, 0}, {3, 0}};
            second_map_type second{{2, 0}, {3, 0}, {4, 0}};
            third_map_type third{{1, 0}, {3, 0}};

            assert(Counter_base::gConstructed == 8);

            first.merge(second);
            first.merge(third);

            assert(map_equal(first, {{1, 0}, {2, 0}, {3, 0}, {4, 0}, {2, 0}, {3, 0}, {1, 0}, {3, 0}}));
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

            assert(map_equal(first, {{1, 0}, {2, 0}, {3, 0}, {4, 0}, {2, 0}, {3, 0}, {1, 0}, {3, 0}}));
            assert(map_equal(second, {}));
            assert(map_equal(third, {}));

            assert(Counter_base::gConstructed == 8);
        }
        assert(Counter_base::gConstructed == 0);
    }
    {
        std::unordered_multimap<int, int> first;
        {
            std::unordered_multimap<int, int> second;
            first.merge(second);
            first.merge(std::move(second));
        }
        {
            std::unordered_map<int, int> second;
            first.merge(second);
            first.merge(std::move(second));
        }
    }
}
