//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14
// <optional>

// void swap(optional&)
//     noexcept(is_nothrow_move_constructible<T>::value &&
//              is_nothrow_swappable<T>::value)

#include <optional>
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "archetypes.hpp"

using std::optional;

class X
{
    int i_;
public:
    static unsigned dtor_called;
    X(int i) : i_(i) {}
    X(X&& x) = default;
    X& operator=(X&&) = default;
    ~X() {++dtor_called;}

    friend bool operator==(const X& x, const X& y) {return x.i_ == y.i_;}
};

unsigned X::dtor_called = 0;

class Y
{
    int i_;
public:
    static unsigned dtor_called;
    Y(int i) : i_(i) {}
    Y(Y&&) = default;
    ~Y() {++dtor_called;}

    friend constexpr bool operator==(const Y& x, const Y& y) {return x.i_ == y.i_;}
    friend void swap(Y& x, Y& y) {std::swap(x.i_, y.i_);}
};

unsigned Y::dtor_called = 0;

class Z
{
    int i_;
public:
    Z(int i) : i_(i) {}
    Z(Z&&) {TEST_THROW(7);}

    friend constexpr bool operator==(const Z& x, const Z& y) {return x.i_ == y.i_;}
    friend void swap(Z&, Z&) {TEST_THROW(6);}
};


int main()
{
    {
        optional<int> opt1;
        optional<int> opt2;
        static_assert(noexcept(opt1.swap(opt2)) == true, "");
        assert(static_cast<bool>(opt1) == false);
        assert(static_cast<bool>(opt2) == false);
        opt1.swap(opt2);
        assert(static_cast<bool>(opt1) == false);
        assert(static_cast<bool>(opt2) == false);
    }
    {
        optional<int> opt1(1);
        optional<int> opt2;
        static_assert(noexcept(opt1.swap(opt2)) == true, "");
        assert(static_cast<bool>(opt1) == true);
        assert(*opt1 == 1);
        assert(static_cast<bool>(opt2) == false);
        opt1.swap(opt2);
        assert(static_cast<bool>(opt1) == false);
        assert(static_cast<bool>(opt2) == true);
        assert(*opt2 == 1);
    }
    {
        optional<int> opt1;
        optional<int> opt2(2);
        static_assert(noexcept(opt1.swap(opt2)) == true, "");
        assert(static_cast<bool>(opt1) == false);
        assert(static_cast<bool>(opt2) == true);
        assert(*opt2 == 2);
        opt1.swap(opt2);
        assert(static_cast<bool>(opt1) == true);
        assert(*opt1 == 2);
        assert(static_cast<bool>(opt2) == false);
    }
    {
        optional<int> opt1(1);
        optional<int> opt2(2);
        static_assert(noexcept(opt1.swap(opt2)) == true, "");
        assert(static_cast<bool>(opt1) == true);
        assert(*opt1 == 1);
        assert(static_cast<bool>(opt2) == true);
        assert(*opt2 == 2);
        opt1.swap(opt2);
        assert(static_cast<bool>(opt1) == true);
        assert(*opt1 == 2);
        assert(static_cast<bool>(opt2) == true);
        assert(*opt2 == 1);
    }
    {
        optional<X> opt1;
        optional<X> opt2;
        static_assert(noexcept(opt1.swap(opt2)) == true, "");
        assert(static_cast<bool>(opt1) == false);
        assert(static_cast<bool>(opt2) == false);
        opt1.swap(opt2);
        assert(static_cast<bool>(opt1) == false);
        assert(static_cast<bool>(opt2) == false);
        assert(X::dtor_called == 0);
    }
    {
        optional<X> opt1(1);
        optional<X> opt2;
        static_assert(noexcept(opt1.swap(opt2)) == true, "");
        assert(static_cast<bool>(opt1) == true);
        assert(*opt1 == 1);
        assert(static_cast<bool>(opt2) == false);
        X::dtor_called = 0;
        opt1.swap(opt2);
        assert(X::dtor_called == 1);
        assert(static_cast<bool>(opt1) == false);
        assert(static_cast<bool>(opt2) == true);
        assert(*opt2 == 1);
    }
    {
        optional<X> opt1;
        optional<X> opt2(2);
        static_assert(noexcept(opt1.swap(opt2)) == true, "");
        assert(static_cast<bool>(opt1) == false);
        assert(static_cast<bool>(opt2) == true);
        assert(*opt2 == 2);
        X::dtor_called = 0;
        opt1.swap(opt2);
        assert(X::dtor_called == 1);
        assert(static_cast<bool>(opt1) == true);
        assert(*opt1 == 2);
        assert(static_cast<bool>(opt2) == false);
    }
    {
        optional<X> opt1(1);
        optional<X> opt2(2);
        static_assert(noexcept(opt1.swap(opt2)) == true, "");
        assert(static_cast<bool>(opt1) == true);
        assert(*opt1 == 1);
        assert(static_cast<bool>(opt2) == true);
        assert(*opt2 == 2);
        X::dtor_called = 0;
        opt1.swap(opt2);
        assert(X::dtor_called == 1);  // from inside std::swap
        assert(static_cast<bool>(opt1) == true);
        assert(*opt1 == 2);
        assert(static_cast<bool>(opt2) == true);
        assert(*opt2 == 1);
    }
    {
        optional<Y> opt1;
        optional<Y> opt2;
        static_assert(noexcept(opt1.swap(opt2)) == false, "");
        assert(static_cast<bool>(opt1) == false);
        assert(static_cast<bool>(opt2) == false);
        opt1.swap(opt2);
        assert(static_cast<bool>(opt1) == false);
        assert(static_cast<bool>(opt2) == false);
        assert(Y::dtor_called == 0);
    }
    {
        optional<Y> opt1(1);
        optional<Y> opt2;
        static_assert(noexcept(opt1.swap(opt2)) == false, "");
        assert(static_cast<bool>(opt1) == true);
        assert(*opt1 == 1);
        assert(static_cast<bool>(opt2) == false);
        Y::dtor_called = 0;
        opt1.swap(opt2);
        assert(Y::dtor_called == 1);
        assert(static_cast<bool>(opt1) == false);
        assert(static_cast<bool>(opt2) == true);
        assert(*opt2 == 1);
    }
    {
        optional<Y> opt1;
        optional<Y> opt2(2);
        static_assert(noexcept(opt1.swap(opt2)) == false, "");
        assert(static_cast<bool>(opt1) == false);
        assert(static_cast<bool>(opt2) == true);
        assert(*opt2 == 2);
        Y::dtor_called = 0;
        opt1.swap(opt2);
        assert(Y::dtor_called == 1);
        assert(static_cast<bool>(opt1) == true);
        assert(*opt1 == 2);
        assert(static_cast<bool>(opt2) == false);
    }
    {
        optional<Y> opt1(1);
        optional<Y> opt2(2);
        static_assert(noexcept(opt1.swap(opt2)) == false, "");
        assert(static_cast<bool>(opt1) == true);
        assert(*opt1 == 1);
        assert(static_cast<bool>(opt2) == true);
        assert(*opt2 == 2);
        Y::dtor_called = 0;
        opt1.swap(opt2);
        assert(Y::dtor_called == 0);
        assert(static_cast<bool>(opt1) == true);
        assert(*opt1 == 2);
        assert(static_cast<bool>(opt2) == true);
        assert(*opt2 == 1);
    }
    {
        optional<Z> opt1;
        optional<Z> opt2;
        static_assert(noexcept(opt1.swap(opt2)) == false, "");
        assert(static_cast<bool>(opt1) == false);
        assert(static_cast<bool>(opt2) == false);
        opt1.swap(opt2);
        assert(static_cast<bool>(opt1) == false);
        assert(static_cast<bool>(opt2) == false);
    }
#ifndef TEST_HAS_NO_EXCEPTIONS
    {
        optional<Z> opt1;
        opt1.emplace(1);
        optional<Z> opt2;
        static_assert(noexcept(opt1.swap(opt2)) == false, "");
        assert(static_cast<bool>(opt1) == true);
        assert(*opt1 == 1);
        assert(static_cast<bool>(opt2) == false);
        try
        {
            opt1.swap(opt2);
            assert(false);
        }
        catch (int i)
        {
            assert(i == 7);
        }
        assert(static_cast<bool>(opt1) == true);
        assert(*opt1 == 1);
        assert(static_cast<bool>(opt2) == false);
    }
    {
        optional<Z> opt1;
        optional<Z> opt2;
        opt2.emplace(2);
        static_assert(noexcept(opt1.swap(opt2)) == false, "");
        assert(static_cast<bool>(opt1) == false);
        assert(static_cast<bool>(opt2) == true);
        assert(*opt2 == 2);
        try
        {
            opt1.swap(opt2);
            assert(false);
        }
        catch (int i)
        {
            assert(i == 7);
        }
        assert(static_cast<bool>(opt1) == false);
        assert(static_cast<bool>(opt2) == true);
        assert(*opt2 == 2);
    }
    {
        optional<Z> opt1;
        opt1.emplace(1);
        optional<Z> opt2;
        opt2.emplace(2);
        static_assert(noexcept(opt1.swap(opt2)) == false, "");
        assert(static_cast<bool>(opt1) == true);
        assert(*opt1 == 1);
        assert(static_cast<bool>(opt2) == true);
        assert(*opt2 == 2);
        try
        {
            opt1.swap(opt2);
            assert(false);
        }
        catch (int i)
        {
            assert(i == 6);
        }
        assert(static_cast<bool>(opt1) == true);
        assert(*opt1 == 1);
        assert(static_cast<bool>(opt2) == true);
        assert(*opt2 == 2);
    }
#endif
}
