//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14

// <unordered_set>

// class unordered_set

// template <class H2, class P2>
//   void merge(unordered_set<key_type, H2, P2, allocator_type>& source);
// template <class H2, class P2>
//   void merge(unordered_set<key_type, H2, P2, allocator_type>&& source);
// template <class H2, class P2>
//   void merge(unordered_multiset<key_type, H2, P2, allocator_type>& source);
// template <class H2, class P2>
//   void merge(unordered_multiset<key_type, H2, P2, allocator_type>&& source);

#include <unordered_set>
#include <cassert>
#include "test_macros.h"
#include "Counter.h"

template <class Set>
bool set_equal(const Set& set, Set other)
{
    return set == other;
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
        std::unordered_set<int> src{1, 3, 5};
        std::unordered_set<int> dst{2, 4, 5};
        dst.merge(src);
        assert(set_equal(src, {5}));
        assert(set_equal(dst, {1, 2, 3, 4, 5}));
    }

#ifndef TEST_HAS_NO_EXCEPTIONS
    {
        bool do_throw = false;
        typedef std::unordered_set<Counter<int>, throw_hasher<Counter<int>>> set_type;
        set_type src({1, 3, 5}, 0, throw_hasher<Counter<int>>(do_throw));
        set_type dst({2, 4, 5}, 0, throw_hasher<Counter<int>>(do_throw));

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
        assert(set_equal(src, set_type({1, 3, 5}, 0, throw_hasher<Counter<int>>(do_throw))));
        assert(set_equal(dst, set_type({2, 4, 5}, 0, throw_hasher<Counter<int>>(do_throw))));
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
        size_t operator()(const Counter<int>& p) const { return std::hash<Counter<int>>()(p); }
    };
    {
        typedef std::unordered_set<Counter<int>, std::hash<Counter<int>>, std::equal_to<Counter<int>>> first_set_type;
        typedef std::unordered_set<Counter<int>, hasher, equal> second_set_type;
        typedef std::unordered_multiset<Counter<int>, hasher, equal> third_set_type;

        {
            first_set_type first{1, 2, 3};
            second_set_type second{2, 3, 4};
            third_set_type third{1, 3};

            assert(Counter_base::gConstructed == 8);

            first.merge(second);
            first.merge(third);

            assert(set_equal(first, {1, 2, 3, 4}));
            assert(set_equal(second, {2, 3}));
            assert(set_equal(third, {1, 3}));

            assert(Counter_base::gConstructed == 8);
        }
        assert(Counter_base::gConstructed == 0);
        {
            first_set_type first{1, 2, 3};
            second_set_type second{2, 3, 4};
            third_set_type third{1, 3};

            assert(Counter_base::gConstructed == 8);

            first.merge(std::move(second));
            first.merge(std::move(third));

            assert(set_equal(first, {1, 2, 3, 4}));
            assert(set_equal(second, {2, 3}));
            assert(set_equal(third, {1, 3}));

            assert(Counter_base::gConstructed == 8);
        }
        assert(Counter_base::gConstructed == 0);
    }
    {
        std::unordered_set<int> first;
        {
            std::unordered_set<int> second;
            first.merge(second);
            first.merge(std::move(second));
        }
        {
            std::unordered_multiset<int> second;
            first.merge(second);
            first.merge(std::move(second));
        }
    }
}
