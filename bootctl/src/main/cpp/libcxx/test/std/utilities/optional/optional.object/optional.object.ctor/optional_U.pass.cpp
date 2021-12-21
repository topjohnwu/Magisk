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
//   optional(optional<U>&& rhs);

#include <optional>
#include <type_traits>
#include <memory>
#include <cassert>

#include "test_macros.h"

using std::optional;

template <class T, class U>
void
test(optional<U>&& rhs, bool is_going_to_throw = false)
{
    bool rhs_engaged = static_cast<bool>(rhs);
#ifndef TEST_HAS_NO_EXCEPTIONS
    try
    {
        optional<T> lhs = std::move(rhs);
        assert(is_going_to_throw == false);
        assert(static_cast<bool>(lhs) == rhs_engaged);
    }
    catch (int i)
    {
        assert(i == 6);
    }
#else
    if (is_going_to_throw) return;
    optional<T> lhs = std::move(rhs);
    assert(static_cast<bool>(lhs) == rhs_engaged);
#endif
}

class X
{
    int i_;
public:
    X(int i) : i_(i) {}
    X(X&& x) : i_(std::exchange(x.i_, 0)) {}
    ~X() {i_ = 0;}
    friend bool operator==(const X& x, const X& y) {return x.i_ == y.i_;}
};

int count = 0;

struct Z
{
    Z(int) { TEST_THROW(6); }
};

int main()
{
    {
        optional<short> rhs;
        test<int>(std::move(rhs));
    }
    {
        optional<short> rhs(short{3});
        test<int>(std::move(rhs));
    }
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

    static_assert(!(std::is_constructible<optional<X>, optional<Z>>::value), "");
}
