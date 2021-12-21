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

// pair(const T1& x, const T2& y);

#include <utility>
#include <cassert>

#include "archetypes.hpp"
#include "test_convertible.hpp"
using namespace ImplicitTypes; // Get implicitly archetypes

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

template <class T1,
          bool CanCopy = true, bool CanConvert = CanCopy>
void test_sfinae() {
    using P1 = std::pair<T1, int>;
    using P2 = std::pair<int, T1>;
    using T1Arg = T1 const&;
    using T2 = int const&;
    static_assert(std::is_constructible<P1, T1Arg, T2>::value == CanCopy, "");
    static_assert(test_convertible<P1,   T1Arg, T2>() == CanConvert, "");
    static_assert(std::is_constructible<P2, T2,   T1Arg>::value == CanCopy, "");
    static_assert(test_convertible<P2,   T2,   T1Arg>() == CanConvert, "");
}

int main()
{
    {
        typedef std::pair<float, short*> P;
        P p(3.5f, 0);
        assert(p.first == 3.5f);
        assert(p.second == nullptr);
    }
    {
        typedef std::pair<ImplicitT, int> P;
        P p(1, 2);
        assert(p.first.value == 1);
        assert(p.second == 2);
    }
    {
        test_sfinae<AllCtors>();
        test_sfinae<ExplicitTypes::AllCtors, true, false>();
        test_sfinae<CopyOnly>();
        test_sfinae<ExplicitTypes::CopyOnly, true, false>();
        test_sfinae<MoveOnly, false>();
        test_sfinae<ExplicitTypes::MoveOnly, false>();
        test_sfinae<NonCopyable, false>();
        test_sfinae<ExplicitTypes::NonCopyable, false>();
    }
#if TEST_STD_VER > 11
    {
        typedef std::pair<float, short*> P;
        constexpr P p(3.5f, 0);
        static_assert(p.first == 3.5f, "");
        static_assert(p.second == nullptr, "");
    }
    {
        using P = std::pair<ExplicitT, int>;
        constexpr ExplicitT e(42);
        constexpr int x = 10;
        constexpr P p(e, x);
        static_assert(p.first.value == 42, "");
        static_assert(p.second == 10, "");
    }
    {
        using P = std::pair<ImplicitT, int>;
        constexpr ImplicitT e(42);
        constexpr int x = 10;
        constexpr P p = {e, x};
        static_assert(p.first.value == 42, "");
        static_assert(p.second == 10, "");
    }
#endif
}
