//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <utility>

// template <class T1, class T2> struct pair

// Test the SFINAE required by LWG Issue #2367.
// is_default_constructible<pair>

// UNSUPPORTED: c++98, c++03

#include <utility>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

#if TEST_STD_VER > 11
#define CONSTEXPR_CXX14 constexpr
#define STATIC_ASSERT_CXX14(Pred) static_assert(Pred, "")
#else
#define CONSTEXPR_CXX14
#define STATIC_ASSERT_CXX14(Pred) assert(Pred)
#endif

struct DeletedDefault {
    // A class with a deleted default constructor. Used to test the SFINAE
    // on std::pair's default constructor.
    constexpr explicit DeletedDefault(int x) : value(x) {}
    constexpr DeletedDefault() = delete;
    int value;
};

template <class Tp, bool>
struct DependantType: public Tp {};

template <class T, bool Val>
using DependantIsDefault = DependantType<std::is_default_constructible<T>, Val>;

template <class T>
struct DefaultSFINAES {
    template <bool Dummy = false, class = typename std::enable_if<
             DependantIsDefault<T, Dummy>::value
                >::type
            >
    constexpr DefaultSFINAES() : value() {}
    constexpr explicit DefaultSFINAES(T const& x) : value(x) {}
    T value;
};

struct NoDefault {
    constexpr NoDefault(int v) : value(v) {}
    int value;
};

template <class Tp>
void test_not_is_default_constructible()
{
    {
        typedef std::pair<int, Tp> P;
        static_assert(!std::is_default_constructible<P>::value, "");
        static_assert(std::is_constructible<P, int, Tp>::value, "");
    }
    {
        typedef std::pair<Tp, int> P;
        static_assert(!std::is_default_constructible<P>::value, "");
        static_assert(std::is_constructible<P, Tp, int>::value, "");
    }
    {
        typedef std::pair<Tp, Tp> P;
        static_assert(!std::is_default_constructible<P>::value, "");
        static_assert(std::is_constructible<P, Tp, Tp>::value, "");
    }
}

template <class Tp>
void test_is_default_constructible()
{
    {
        typedef std::pair<int, Tp> P;
        static_assert(std::is_default_constructible<P>::value, "");
    }
    {
        typedef std::pair<Tp, int> P;
        static_assert(std::is_default_constructible<P>::value, "");
    }
    {
        typedef std::pair<Tp, Tp> P;
        static_assert(std::is_default_constructible<P>::value, "");
    }
}

template <class T>
struct IllFormedDefaultImp {
  constexpr explicit IllFormedDefaultImp(int v) : value(v) {}
  constexpr IllFormedDefaultImp() : value(T::DoesNotExistAndShouldNotCompile) {}
  int value;
};

typedef IllFormedDefaultImp<int> IllFormedDefault;
    // A class which provides a constexpr default constructor with a valid
    // signature but an ill-formed body. The A compile error will be emitted if
    // the default constructor is instantiated.


// Check that the SFINAE on the default constructor is not evaluated when
// it isn't needed. If the default constructor of 'IllFormedDefault' is evaluated
// in C++11, even with is_default_constructible, then this test should fail to
// compile. In C++14 and greater evaluate each test is evaluated as a constant
// expression.
// See LWG issue #2367
void test_illformed_default()
{
    {
    typedef std::pair<IllFormedDefault, int> P;
    static_assert((std::is_constructible<P, IllFormedDefault, int>::value), "");
    CONSTEXPR_CXX14 P p(IllFormedDefault(42), -5);
    STATIC_ASSERT_CXX14(p.first.value == 42 && p.second == -5);
    }
    {
    typedef std::pair<int, IllFormedDefault> P;
    static_assert((std::is_constructible<P, int, IllFormedDefault>::value), "");
    CONSTEXPR_CXX14 IllFormedDefault dd(-5);
    CONSTEXPR_CXX14 P p(42, dd);
    STATIC_ASSERT_CXX14(p.first == 42 && p.second.value == -5);
    }
    {
    typedef std::pair<IllFormedDefault, IllFormedDefault> P;
    static_assert((std::is_constructible<P, IllFormedDefault, IllFormedDefault>::value), "");
    CONSTEXPR_CXX14 P p(IllFormedDefault(42), IllFormedDefault(-5));
    STATIC_ASSERT_CXX14(p.first.value == 42 && p.second.value == -5);
    }
}


int main()
{
    {
        // Check that pair<T, U> can still be used even if
        // is_default_constructible<T> or is_default_constructible<U> cause
        // a compilation error.
        test_illformed_default();
    }
    {
        // pair::pair() is only disable in C++11 and beyond.
        test_not_is_default_constructible<NoDefault>();
        test_not_is_default_constructible<DeletedDefault>();
        test_not_is_default_constructible<DefaultSFINAES<int&>>();
        test_not_is_default_constructible<DefaultSFINAES<int&&>>();
        test_not_is_default_constructible<int&>();
        test_not_is_default_constructible<int&&>();
    }
    {
        test_is_default_constructible<int>();
        test_is_default_constructible<DefaultSFINAES<int>>();
    }
}
