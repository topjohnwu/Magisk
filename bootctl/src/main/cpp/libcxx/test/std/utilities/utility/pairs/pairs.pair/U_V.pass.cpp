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

// template<class U, class V> pair(U&& x, V&& y);


#include <utility>
#include <memory>
#include <cassert>

#include "archetypes.hpp"
#include "test_convertible.hpp"
using namespace ImplicitTypes; // Get implicitly archetypes

template <class T1, class T1Arg,
          bool CanCopy = true, bool CanConvert = CanCopy>
void test_sfinae() {
    using P1 = std::pair<T1, int>;
    using P2 = std::pair<int, T1>;
    using T2 = int const&;
    static_assert(std::is_constructible<P1, T1Arg, T2>::value == CanCopy, "");
    static_assert(test_convertible<P1,   T1Arg, T2>() == CanConvert, "");
    static_assert(std::is_constructible<P2, T2,   T1Arg>::value == CanCopy, "");
    static_assert(test_convertible<P2,   T2,   T1Arg>() == CanConvert, "");
}

struct ExplicitT {
  constexpr explicit ExplicitT(int x) : value(x) {}
  int value;
};

struct ImplicitT {
  constexpr ImplicitT(int x) : value(x) {}
  int value;
};


int main()
{
    {
        typedef std::pair<std::unique_ptr<int>, short*> P;
        P p(std::unique_ptr<int>(new int(3)), nullptr);
        assert(*p.first == 3);
        assert(p.second == nullptr);
    }
    {
        // Test non-const lvalue and rvalue types
        test_sfinae<AllCtors, AllCtors&>();
        test_sfinae<AllCtors, AllCtors&&>();
        test_sfinae<ExplicitTypes::AllCtors, ExplicitTypes::AllCtors&, true, false>();
        test_sfinae<ExplicitTypes::AllCtors, ExplicitTypes::AllCtors&&, true, false>();
        test_sfinae<CopyOnly, CopyOnly&>();
        test_sfinae<CopyOnly, CopyOnly&&>();
        test_sfinae<ExplicitTypes::CopyOnly, ExplicitTypes::CopyOnly&, true, false>();
        test_sfinae<ExplicitTypes::CopyOnly, ExplicitTypes::CopyOnly&&, true, false>();
        test_sfinae<MoveOnly, MoveOnly&, false>();
        test_sfinae<MoveOnly, MoveOnly&&>();
        test_sfinae<ExplicitTypes::MoveOnly, ExplicitTypes::MoveOnly&, false>();
        test_sfinae<ExplicitTypes::MoveOnly, ExplicitTypes::MoveOnly&&, true, false>();
        test_sfinae<NonCopyable, NonCopyable&, false>();
        test_sfinae<NonCopyable, NonCopyable&&, false>();
        test_sfinae<ExplicitTypes::NonCopyable, ExplicitTypes::NonCopyable&, false>();
        test_sfinae<ExplicitTypes::NonCopyable, ExplicitTypes::NonCopyable&&, false>();
    }
    {
        // Test converting types
        test_sfinae<ConvertingType, int&>();
        test_sfinae<ConvertingType, const int&>();
        test_sfinae<ConvertingType, int&&>();
        test_sfinae<ConvertingType, const int&&>();
        test_sfinae<ExplicitTypes::ConvertingType, int&, true, false>();
        test_sfinae<ExplicitTypes::ConvertingType, const int&, true, false>();
        test_sfinae<ExplicitTypes::ConvertingType, int&&, true, false>();
        test_sfinae<ExplicitTypes::ConvertingType, const int&&, true, false>();
    }
#if TEST_STD_VER > 11
    { // explicit constexpr test
        constexpr std::pair<ExplicitT, ExplicitT> p(42, 43);
        static_assert(p.first.value == 42, "");
        static_assert(p.second.value == 43, "");
    }
    { // implicit constexpr test
        constexpr std::pair<ImplicitT, ImplicitT> p = {42, 43};
        static_assert(p.first.value == 42, "");
        static_assert(p.second.value == 43, "");
    }
#endif
}
