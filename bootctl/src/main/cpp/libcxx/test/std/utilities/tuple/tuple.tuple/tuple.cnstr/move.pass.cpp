//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <tuple>

// template <class... Types> class tuple;

// tuple(tuple&& u);

// UNSUPPORTED: c++98, c++03

#include <tuple>
#include <utility>
#include <cassert>

#include "MoveOnly.h"

struct ConstructsWithTupleLeaf
{
    ConstructsWithTupleLeaf() {}

    ConstructsWithTupleLeaf(ConstructsWithTupleLeaf const &) { assert(false); }
    ConstructsWithTupleLeaf(ConstructsWithTupleLeaf &&) {}

    template <class T>
    ConstructsWithTupleLeaf(T) {
        static_assert(!std::is_same<T, T>::value,
                      "Constructor instantiated for type other than int");
    }
};

// move_only type which triggers the empty base optimization
struct move_only_ebo {
  move_only_ebo() = default;
  move_only_ebo(move_only_ebo&&) = default;
};

// a move_only type which does not trigger the empty base optimization
struct move_only_large final {
  move_only_large() : value(42) {}
  move_only_large(move_only_large&&) = default;
  int value;
};

template <class Elem>
void test_sfinae() {
    using Tup = std::tuple<Elem>;
    using Alloc = std::allocator<void>;
    using Tag = std::allocator_arg_t;
    // special members
    {
        static_assert(std::is_default_constructible<Tup>::value, "");
        static_assert(std::is_move_constructible<Tup>::value, "");
        static_assert(!std::is_copy_constructible<Tup>::value, "");
        static_assert(!std::is_constructible<Tup, Tup&>::value, "");
    }
    // args constructors
    {
        static_assert(std::is_constructible<Tup, Elem&&>::value, "");
        static_assert(!std::is_constructible<Tup, Elem const&>::value, "");
        static_assert(!std::is_constructible<Tup, Elem&>::value, "");
    }
    // uses-allocator special member constructors
    {
        static_assert(std::is_constructible<Tup, Tag, Alloc>::value, "");
        static_assert(std::is_constructible<Tup, Tag, Alloc, Tup&&>::value, "");
        static_assert(!std::is_constructible<Tup, Tag, Alloc, Tup const&>::value, "");
        static_assert(!std::is_constructible<Tup, Tag, Alloc, Tup &>::value, "");
    }
    // uses-allocator args constructors
    {
        static_assert(std::is_constructible<Tup, Tag, Alloc, Elem&&>::value, "");
        static_assert(!std::is_constructible<Tup, Tag, Alloc, Elem const&>::value, "");
        static_assert(!std::is_constructible<Tup, Tag, Alloc, Elem &>::value, "");
    }
}

int main()
{
    {
        typedef std::tuple<> T;
        T t0;
        T t = std::move(t0);
        ((void)t); // Prevent unused warning
    }
    {
        typedef std::tuple<MoveOnly> T;
        T t0(MoveOnly(0));
        T t = std::move(t0);
        assert(std::get<0>(t) == 0);
    }
    {
        typedef std::tuple<MoveOnly, MoveOnly> T;
        T t0(MoveOnly(0), MoveOnly(1));
        T t = std::move(t0);
        assert(std::get<0>(t) == 0);
        assert(std::get<1>(t) == 1);
    }
    {
        typedef std::tuple<MoveOnly, MoveOnly, MoveOnly> T;
        T t0(MoveOnly(0), MoveOnly(1), MoveOnly(2));
        T t = std::move(t0);
        assert(std::get<0>(t) == 0);
        assert(std::get<1>(t) == 1);
        assert(std::get<2>(t) == 2);
    }
    // A bug in tuple caused __tuple_leaf to use its explicit converting constructor
    //  as its move constructor. This tests that ConstructsWithTupleLeaf is not called
    // (w/ __tuple_leaf)
    {
        typedef std::tuple<ConstructsWithTupleLeaf> d_t;
        d_t d((ConstructsWithTupleLeaf()));
        d_t d2(static_cast<d_t &&>(d));
    }
    {
        test_sfinae<move_only_ebo>();
        test_sfinae<move_only_large>();
    }
}
