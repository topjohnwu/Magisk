//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <utility>

// template <class T1, class T2> struct pair

// template <class U, class V> EXPLICIT constexpr pair(const pair<U, V>& p);

#include <utility>
#include <cassert>

#include "archetypes.hpp"
#include "test_convertible.hpp"
using namespace ImplicitTypes; // Get implicitly archetypes

template <class T1, class U1,
          bool CanCopy = true, bool CanConvert = CanCopy>
void test_pair_const()
{
    using P1 = std::pair<T1, int>;
    using P2 = std::pair<int, T1>;
    using UP1 = std::pair<U1, int> const&;
    using UP2 = std::pair<int, U1> const&;
    static_assert(std::is_constructible<P1, UP1>::value == CanCopy, "");
    static_assert(test_convertible<P1, UP1>() == CanConvert, "");
    static_assert(std::is_constructible<P2, UP2>::value == CanCopy, "");
    static_assert(test_convertible<P2,  UP2>() == CanConvert, "");
}

template <class T, class U>
struct DPair : public std::pair<T, U> {
  using Base = std::pair<T, U>;
  using Base::Base;
};

struct ExplicitT {
  constexpr explicit ExplicitT(int x) : value(x) {}
  constexpr explicit ExplicitT(ExplicitT const& o) : value(o.value) {}
  int value;
};

struct ImplicitT {
  constexpr ImplicitT(int x) : value(x) {}
  constexpr ImplicitT(ImplicitT const& o) : value(o.value) {}
  int value;
};

int main()
{
    {
        typedef std::pair<int, int> P1;
        typedef std::pair<double, long> P2;
        const P1 p1(3, 4);
        const P2 p2 = p1;
        assert(p2.first == 3);
        assert(p2.second == 4);
    }
    {
        // We allow derived types to use this constructor
        using P1 = DPair<long, long>;
        using P2 = std::pair<int, int>;
        P1 p1(42, 101);
        P2 p2(p1);
        assert(p2.first == 42);
        assert(p2.second == 101);
    }
    {
        test_pair_const<AllCtors, AllCtors>(); // copy construction
        test_pair_const<AllCtors, AllCtors&>();
        test_pair_const<AllCtors, AllCtors&&>();
        test_pair_const<AllCtors, const AllCtors&>();
        test_pair_const<AllCtors, const AllCtors&&>();

        test_pair_const<ExplicitTypes::AllCtors, ExplicitTypes::AllCtors>(); // copy construction
        test_pair_const<ExplicitTypes::AllCtors, ExplicitTypes::AllCtors&, true, false>();
        test_pair_const<ExplicitTypes::AllCtors, ExplicitTypes::AllCtors&&, true, false>();
        test_pair_const<ExplicitTypes::AllCtors, const ExplicitTypes::AllCtors&, true, false>();
        test_pair_const<ExplicitTypes::AllCtors, const ExplicitTypes::AllCtors&&, true, false>();

        test_pair_const<MoveOnly, MoveOnly, false>(); // copy construction
        test_pair_const<MoveOnly, MoveOnly&, false>();
        test_pair_const<MoveOnly, MoveOnly&&, false>();

        test_pair_const<ExplicitTypes::MoveOnly, ExplicitTypes::MoveOnly, false>(); // copy construction
        test_pair_const<ExplicitTypes::MoveOnly, ExplicitTypes::MoveOnly&, false>();
        test_pair_const<ExplicitTypes::MoveOnly, ExplicitTypes::MoveOnly&&, false>();

        test_pair_const<CopyOnly, CopyOnly>();
        test_pair_const<CopyOnly, CopyOnly&>();
        test_pair_const<CopyOnly, CopyOnly&&>();

        test_pair_const<ExplicitTypes::CopyOnly, ExplicitTypes::CopyOnly>();
        test_pair_const<ExplicitTypes::CopyOnly, ExplicitTypes::CopyOnly&, true, false>();
        test_pair_const<ExplicitTypes::CopyOnly, ExplicitTypes::CopyOnly&&, true, false>();

        test_pair_const<NonCopyable, NonCopyable, false>();
        test_pair_const<NonCopyable, NonCopyable&, false>();
        test_pair_const<NonCopyable, NonCopyable&&, false>();
        test_pair_const<NonCopyable, const NonCopyable&, false>();
        test_pair_const<NonCopyable, const NonCopyable&&, false>();
    }

    { // Test construction of references
        test_pair_const<NonCopyable&, NonCopyable&>();
        test_pair_const<NonCopyable&, NonCopyable&&>();
        test_pair_const<NonCopyable&, NonCopyable const&, false>();
        test_pair_const<NonCopyable const&, NonCopyable&&>();
        test_pair_const<NonCopyable&&, NonCopyable&&, false>();

        test_pair_const<ConvertingType&, int, false>();
        test_pair_const<ExplicitTypes::ConvertingType&, int, false>();
        // Unfortunately the below conversions are allowed and create dangling
        // references.
        //test_pair_const<ConvertingType&&, int>();
        //test_pair_const<ConvertingType const&, int>();
        //test_pair_const<ConvertingType const&&, int>();
        // But these are not because the converting constructor is explicit.
        test_pair_const<ExplicitTypes::ConvertingType&&, int, false>();
        test_pair_const<ExplicitTypes::ConvertingType const&, int, false>();
        test_pair_const<ExplicitTypes::ConvertingType const&&, int, false>();

    }
    {
        test_pair_const<AllCtors, int, false>();
        test_pair_const<ExplicitTypes::AllCtors, int, false>();
        test_pair_const<ConvertingType, int>();
        test_pair_const<ExplicitTypes::ConvertingType, int, true, false>();

        test_pair_const<ConvertingType, int>();
        test_pair_const<ConvertingType, ConvertingType>();
        test_pair_const<ConvertingType, ConvertingType const&>();
        test_pair_const<ConvertingType, ConvertingType&>();
        test_pair_const<ConvertingType, ConvertingType&&>();

        test_pair_const<ExplicitTypes::ConvertingType, int, true, false>();
        test_pair_const<ExplicitTypes::ConvertingType, int&, true, false>();
        test_pair_const<ExplicitTypes::ConvertingType, const int&, true, false>();
        test_pair_const<ExplicitTypes::ConvertingType, int&&, true, false>();
        test_pair_const<ExplicitTypes::ConvertingType, const int&&, true, false>();

        test_pair_const<ExplicitTypes::ConvertingType, ExplicitTypes::ConvertingType>();
        test_pair_const<ExplicitTypes::ConvertingType, ExplicitTypes::ConvertingType const&, true, false>();
        test_pair_const<ExplicitTypes::ConvertingType, ExplicitTypes::ConvertingType&, true, false>();
        test_pair_const<ExplicitTypes::ConvertingType, ExplicitTypes::ConvertingType&&, true, false>();
    }
#if TEST_STD_VER > 11
    {
        typedef std::pair<int, int> P1;
        typedef std::pair<double, long> P2;
        constexpr P1 p1(3, 4);
        constexpr P2 p2 = p1;
        static_assert(p2.first == 3, "");
        static_assert(p2.second == 4, "");
    }
    {
        using P1 = std::pair<int, int>;
        using P2 = std::pair<ExplicitT, ExplicitT>;
        constexpr P1 p1(42, 101);
        constexpr P2 p2(p1);
        static_assert(p2.first.value == 42, "");
        static_assert(p2.second.value == 101, "");
    }
    {
        using P1 = std::pair<int, int>;
        using P2 = std::pair<ImplicitT, ImplicitT>;
        constexpr P1 p1(42, 101);
        constexpr P2 p2 = p1;
        static_assert(p2.first.value == 42, "");
        static_assert(p2.second.value == 101, "");
    }
#endif
}
