//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <memory>

// UNSUPPORTED: c++98, c++03, c++11, c++14, c++17

// template <class T> constexpr T* to_address(T* p) noexcept;
// template <class Ptr> auto to_address(const Ptr& p) noexcept;

#include <memory>
#include <cassert>
#include "test_macros.h"

class P1
{
public:
    using element_type = int;

    explicit P1(int* p)
    : p_(p) { }

    int* operator->() const noexcept
    { return p_; }

private:
    int* p_;
};

class P2
{
public:
    using element_type = int;

    explicit P2(int* p)
    : p_(p) { }

    P1 operator->() const noexcept
    { return p_; }

private:
    P1 p_;
};

class P3
{
public:
    explicit P3(int* p)
    : p_(p) { }

    int* get() const noexcept
    { return p_; }

private:
    int* p_;
};

namespace std
{
template<>
struct pointer_traits<::P3>
{
    static int* to_address(const ::P3& p) noexcept
    { return p.get(); }
};
}

class P4
{
public:
    explicit P4(int* p)
    : p_(p) { }

    int* operator->() const noexcept
    { return nullptr; }

    int* get() const noexcept
    { return p_; }

private:
    int* p_;
};

namespace std
{
template<>
struct pointer_traits<::P4>
{
    static int* to_address(const ::P4& p) noexcept
    { return p.get(); }
};
}

int n = 0;
static_assert(std::to_address(&n) == &n);

int main()
{
    int i = 0;
    ASSERT_NOEXCEPT(std::to_address(&i));
    assert(std::to_address(&i) == &i);
    P1 p1(&i);
    ASSERT_NOEXCEPT(std::to_address(p1));
    assert(std::to_address(p1) == &i);
    P2 p2(&i);
    ASSERT_NOEXCEPT(std::to_address(p2));
    assert(std::to_address(p2) == &i);
    P3 p3(&i);
    ASSERT_NOEXCEPT(std::to_address(p3));
    assert(std::to_address(p3) == &i);
    P4 p4(&i);
    ASSERT_NOEXCEPT(std::to_address(p4));
    assert(std::to_address(p4) == &i);
}
