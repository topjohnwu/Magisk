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
//   explicit optional(optional<U>&& rhs);

#include <optional>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

using std::optional;

template <class T, class U>
void
test(optional<U>&& rhs, bool is_going_to_throw = false)
{
    static_assert(!(std::is_convertible<optional<U>&&, optional<T>>::value), "");
    bool rhs_engaged = static_cast<bool>(rhs);
#ifndef TEST_HAS_NO_EXCEPTIONS
    try
    {
        optional<T> lhs(std::move(rhs));
        assert(is_going_to_throw == false);
        assert(static_cast<bool>(lhs) == rhs_engaged);
    }
    catch (int i)
    {
        assert(i == 6);
    }
#else
    if (is_going_to_throw) return;
    optional<T> lhs(std::move(rhs));
    assert(static_cast<bool>(lhs) == rhs_engaged);
#endif
}

class X
{
    int i_;
public:
    explicit X(int i) : i_(i) {}
    X(X&& x) : i_(std::exchange(x.i_, 0)) {}
    ~X() {i_ = 0;}
    friend bool operator==(const X& x, const X& y) {return x.i_ == y.i_;}
};

int count = 0;

class Z
{
public:
    explicit Z(int) { TEST_THROW(6); }
};

int main()
{
    {
        optional<int> rhs;
        test<X>(std::move(rhs));
    }
    {
        optional<int> rhs(3);
        test<X>(std::move(rhs));
    }
    {
        optional<int> rhs;
        test<Z>(std::move(rhs));
    }
    {
        optional<int> rhs(3);
        test<Z>(std::move(rhs), true);
    }
}
