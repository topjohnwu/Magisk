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

// explicit tuple(const T&...);

// UNSUPPORTED: c++98, c++03

#include <tuple>
#include <string>
#include <cassert>

#include "test_macros.h"

template <class ...>
struct never {
    enum { value = 0 };
};

struct NoValueCtor
{
    NoValueCtor() : id(++count) {}
    NoValueCtor(NoValueCtor const & other) : id(other.id) { ++count; }

    // The constexpr is required to make is_constructible instantiate this template.
    // The explicit is needed to test-around a similar bug with is_convertible.
    template <class T>
    constexpr explicit NoValueCtor(T)
    { static_assert(never<T>::value, "This should not be instantiated"); }

    static int count;
    int id;
};

int NoValueCtor::count = 0;


struct NoValueCtorEmpty
{
    NoValueCtorEmpty() {}
    NoValueCtorEmpty(NoValueCtorEmpty const &) {}

    template <class T>
    constexpr explicit NoValueCtorEmpty(T)
    { static_assert(never<T>::value, "This should not be instantiated"); }
};


struct ImplicitCopy {
  explicit ImplicitCopy(int) {}
  ImplicitCopy(ImplicitCopy const&) {}
};

// Test that tuple(std::allocator_arg, Alloc, Types const&...) allows implicit
// copy conversions in return value expressions.
std::tuple<ImplicitCopy> testImplicitCopy1() {
    ImplicitCopy i(42);
    return {i};
}

std::tuple<ImplicitCopy> testImplicitCopy2() {
    const ImplicitCopy i(42);
    return {i};
}

std::tuple<ImplicitCopy> testImplicitCopy3() {
    const ImplicitCopy i(42);
    return i;
}

int main()
{
    {
        // check that the literal '0' can implicitly initialize a stored pointer.
        std::tuple<int*> t = 0;
        assert(std::get<0>(t) == nullptr);
    }
    {
        std::tuple<int> t(2);
        assert(std::get<0>(t) == 2);
    }
#if TEST_STD_VER > 11
    {
        constexpr std::tuple<int> t(2);
        static_assert(std::get<0>(t) == 2, "");
    }
    {
        constexpr std::tuple<int> t;
        static_assert(std::get<0>(t) == 0, "");
    }
#endif
    {
        std::tuple<int, char*> t(2, 0);
        assert(std::get<0>(t) == 2);
        assert(std::get<1>(t) == nullptr);
    }
#if TEST_STD_VER > 11
    {
        constexpr std::tuple<int, char*> t(2, nullptr);
        static_assert(std::get<0>(t) == 2, "");
        static_assert(std::get<1>(t) == nullptr, "");
    }
#endif
    {
        std::tuple<int, char*> t(2, nullptr);
        assert(std::get<0>(t) == 2);
        assert(std::get<1>(t) == nullptr);
    }
    {
        std::tuple<int, char*, std::string> t(2, nullptr, "text");
        assert(std::get<0>(t) == 2);
        assert(std::get<1>(t) == nullptr);
        assert(std::get<2>(t) == "text");
    }
    // __tuple_leaf<T> uses is_constructible<T, U> to disable its explicit converting
    // constructor overload __tuple_leaf(U &&). Evaluating is_constructible can cause a compile error.
    // This overload is evaluated when __tuple_leafs copy or move ctor is called.
    // This checks that is_constructible is not evaluated when U == __tuple_leaf.
    {
        std::tuple<int, NoValueCtor, int, int> t(1, NoValueCtor(), 2, 3);
        assert(std::get<0>(t) == 1);
        assert(std::get<1>(t).id == 1);
        assert(std::get<2>(t) == 2);
        assert(std::get<3>(t) == 3);
    }
    {
        std::tuple<int, NoValueCtorEmpty, int, int> t(1, NoValueCtorEmpty(), 2, 3);
        assert(std::get<0>(t) == 1);
        assert(std::get<2>(t) == 2);
        assert(std::get<3>(t) == 3);
    }
// extensions
#ifdef _LIBCPP_VERSION
    {
        std::tuple<int, char*, std::string> t(2);
        assert(std::get<0>(t) == 2);
        assert(std::get<1>(t) == nullptr);
        assert(std::get<2>(t) == "");
    }
    {
        std::tuple<int, char*, std::string> t(2, nullptr);
        assert(std::get<0>(t) == 2);
        assert(std::get<1>(t) == nullptr);
        assert(std::get<2>(t) == "");
    }
    {
        std::tuple<int, char*, std::string, double> t(2, nullptr, "text");
        assert(std::get<0>(t) == 2);
        assert(std::get<1>(t) == nullptr);
        assert(std::get<2>(t) == "text");
        assert(std::get<3>(t) == 0.0);
    }
#endif
}
