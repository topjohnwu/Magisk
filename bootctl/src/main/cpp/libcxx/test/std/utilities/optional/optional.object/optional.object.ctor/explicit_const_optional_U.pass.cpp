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

// template <class U>
//   explicit optional(const optional<U>& rhs);

#include <optional>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

using std::optional;

template <class T, class U>
void
test(const optional<U>& rhs, bool is_going_to_throw = false)
{
    static_assert(!(std::is_convertible<const optional<U>&, optional<T>>::value), "");
    bool rhs_engaged = static_cast<bool>(rhs);
#ifndef TEST_HAS_NO_EXCEPTIONS
    try
    {
        optional<T> lhs(rhs);
        assert(is_going_to_throw == false);
        assert(static_cast<bool>(lhs) == rhs_engaged);
        if (rhs_engaged)
            assert(*lhs == T(*rhs));
    }
    catch (int i)
    {
        assert(i == 6);
    }
#else
    if (is_going_to_throw) return;
    optional<T> lhs(rhs);
    assert(static_cast<bool>(lhs) == rhs_engaged);
    if (rhs_engaged)
        assert(*lhs == T(*rhs));
#endif
}

class X
{
    int i_;
public:
    explicit X(int i) : i_(i) {}
    X(const X& x) : i_(x.i_) {}
    ~X() {i_ = 0;}
    friend bool operator==(const X& x, const X& y) {return x.i_ == y.i_;}
};

class Y
{
    int i_;
public:
    explicit Y(int i) : i_(i) {}

    friend constexpr bool operator==(const Y& x, const Y& y) {return x.i_ == y.i_;}
};

int count = 0;

class Z
{
    int i_;
public:
    explicit Z(int i) : i_(i) { TEST_THROW(6);}

    friend constexpr bool operator==(const Z& x, const Z& y) {return x.i_ == y.i_;}
};


int main()
{
    {
        typedef X T;
        typedef int U;
        optional<U> rhs;
        test<T>(rhs);
    }
    {
        typedef X T;
        typedef int U;
        optional<U> rhs(3);
        test<T>(rhs);
    }
    {
        typedef Y T;
        typedef int U;
        optional<U> rhs;
        test<T>(rhs);
    }
    {
        typedef Y T;
        typedef int U;
        optional<U> rhs(3);
        test<T>(rhs);
    }
    {
        typedef Z T;
        typedef int U;
        optional<U> rhs;
        test<T>(rhs);
    }
    {
        typedef Z T;
        typedef int U;
        optional<U> rhs(3);
        test<T>(rhs, true);
    }
}
