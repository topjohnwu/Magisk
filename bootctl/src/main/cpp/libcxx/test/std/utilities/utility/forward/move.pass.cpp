//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test move

// UNSUPPORTED: c++98, c++03

#include <utility>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

class move_only
{
    move_only(const move_only&);
    move_only& operator=(const move_only&);
public:
    move_only(move_only&&) {}
    move_only& operator=(move_only&&) {return *this;}

    move_only() {}
};

move_only source() {return move_only();}
const move_only csource() {return move_only();}

void test(move_only) {}

int x = 42;
const int& cx = x;

template <class QualInt>
QualInt get() noexcept { return static_cast<QualInt>(x); }


int copy_ctor = 0;
int move_ctor = 0;

struct A {
    A() {}
    A(const A&) {++copy_ctor;}
    A(A&&) {++move_ctor;}
    A& operator=(const A&) = delete;
};

constexpr bool test_constexpr_move() {
#if TEST_STD_VER > 11
    int y = 42;
    const int cy = y;
    return std::move(y) == 42
        && std::move(cy) == 42
        && std::move(static_cast<int&&>(y)) == 42
        && std::move(static_cast<int const&&>(y)) == 42;
#else
    return true;
#endif
}

int main()
{
    { // Test return type and noexcept.
        static_assert(std::is_same<decltype(std::move(x)), int&&>::value, "");
        static_assert(noexcept(std::move(x)), "");
        static_assert(std::is_same<decltype(std::move(cx)), const int&&>::value, "");
        static_assert(noexcept(std::move(cx)), "");
        static_assert(std::is_same<decltype(std::move(42)), int&&>::value, "");
        static_assert(noexcept(std::move(42)), "");
        static_assert(std::is_same<decltype(std::move(get<const int&&>())), const int&&>::value, "");
        static_assert(noexcept(std::move(get<int const&&>())), "");
    }
    { // test copy and move semantics
        A a;
        const A ca = A();

        assert(copy_ctor == 0);
        assert(move_ctor == 0);

        A a2 = a;
        assert(copy_ctor == 1);
        assert(move_ctor == 0);

        A a3 = std::move(a);
        assert(copy_ctor == 1);
        assert(move_ctor == 1);

        A a4 = ca;
        assert(copy_ctor == 2);
        assert(move_ctor == 1);

        A a5 = std::move(ca);
        assert(copy_ctor == 3);
        assert(move_ctor == 1);
    }
    { // test on a move only type
        move_only mo;
        test(std::move(mo));
        test(source());
    }
#if TEST_STD_VER > 11
    {
        constexpr int y = 42;
        static_assert(std::move(y) == 42, "");
        static_assert(test_constexpr_move(), "");
    }
#endif
#if TEST_STD_VER == 11 && defined(_LIBCPP_VERSION)
    // Test that std::forward is constexpr in C++11. This is an extension
    // provided by both libc++ and libstdc++.
    {
        constexpr int y = 42;
        static_assert(std::move(y) == 42, "");
    }
#endif
}
