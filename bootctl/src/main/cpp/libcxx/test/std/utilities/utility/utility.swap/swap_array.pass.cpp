//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <utility>

// template<ValueType T, size_t N>
//   requires Swappable<T>
//   void
//   swap(T (&a)[N], T (&b)[N]);

#include <utility>
#include <cassert>
#include <memory>

#include "test_macros.h"


#if TEST_STD_VER >= 11
struct CopyOnly {
    CopyOnly() {}
    CopyOnly(CopyOnly const&) noexcept {}
    CopyOnly& operator=(CopyOnly const&) { return *this; }
};


struct NoexceptMoveOnly {
    NoexceptMoveOnly() {}
    NoexceptMoveOnly(NoexceptMoveOnly&&) noexcept {}
    NoexceptMoveOnly& operator=(NoexceptMoveOnly&&) noexcept { return *this; }
};

struct NotMoveConstructible {
    NotMoveConstructible() {}
    NotMoveConstructible& operator=(NotMoveConstructible&&) { return *this; }
private:
    NotMoveConstructible(NotMoveConstructible&&);
};

template <class Tp>
auto can_swap_test(int) -> decltype(std::swap(std::declval<Tp>(), std::declval<Tp>()));

template <class Tp>
auto can_swap_test(...) -> std::false_type;

template <class Tp>
constexpr bool can_swap() {
    return std::is_same<decltype(can_swap_test<Tp>(0)), void>::value;
}
#endif


int main()
{
    {
        int i[3] = {1, 2, 3};
        int j[3] = {4, 5, 6};
        std::swap(i, j);
        assert(i[0] == 4);
        assert(i[1] == 5);
        assert(i[2] == 6);
        assert(j[0] == 1);
        assert(j[1] == 2);
        assert(j[2] == 3);
    }
#if TEST_STD_VER >= 11
    {
        std::unique_ptr<int> i[3];
        for (int k = 0; k < 3; ++k)
            i[k].reset(new int(k+1));
        std::unique_ptr<int> j[3];
        for (int k = 0; k < 3; ++k)
            j[k].reset(new int(k+4));
        std::swap(i, j);
        assert(*i[0] == 4);
        assert(*i[1] == 5);
        assert(*i[2] == 6);
        assert(*j[0] == 1);
        assert(*j[1] == 2);
        assert(*j[2] == 3);
    }
    {
        using CA = CopyOnly[42];
        using MA = NoexceptMoveOnly[42];
        using NA = NotMoveConstructible[42];
        static_assert(can_swap<CA&>(), "");
        static_assert(can_swap<MA&>(), "");
        static_assert(!can_swap<NA&>(), "");

        CA ca;
        MA ma;
        static_assert(!noexcept(std::swap(ca, ca)), "");
        static_assert(noexcept(std::swap(ma, ma)), "");
    }
#endif
}
