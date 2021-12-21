//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits
// XFAIL: apple-clang-6.0
//  The Apple-6 compiler gets is_constructible<void ()> wrong.

// template <class T, class... Args>
//   struct is_constructible;

// MODULES_DEFINES: _LIBCPP_TESTING_FALLBACK_IS_CONSTRUCTIBLE
#define _LIBCPP_TESTING_FALLBACK_IS_CONSTRUCTIBLE
#include <type_traits>
#include "test_macros.h"

#if TEST_STD_VER >= 11 && defined(_LIBCPP_VERSION)
#define LIBCPP11_STATIC_ASSERT(...) static_assert(__VA_ARGS__)
#else
#define LIBCPP11_STATIC_ASSERT(...) ((void)0)
#endif


struct A
{
    explicit A(int);
    A(int, double);
    A(int, long, double);
#if TEST_STD_VER >= 11
private:
#endif
    A(char);
};

struct Base {};
struct Derived : public Base {};

class Abstract
{
    virtual void foo() = 0;
};

class AbstractDestructor
{
    virtual ~AbstractDestructor() = 0;
};

struct PrivateDtor {
  PrivateDtor(int) {}
private:
  ~PrivateDtor() {}
};

struct S {
   template <class T>
#if TEST_STD_VER >= 11
   explicit
#endif
   operator T () const;
};

template <class To>
struct ImplicitTo {
  operator To();
};

#if TEST_STD_VER >= 11
template <class To>
struct ExplicitTo {
   explicit operator To ();
};
#endif


template <class T>
void test_is_constructible()
{
    static_assert( (std::is_constructible<T>::value), "");
    LIBCPP11_STATIC_ASSERT((std::__libcpp_is_constructible<T>::type::value), "");
#if TEST_STD_VER > 14
    static_assert( std::is_constructible_v<T>, "");
#endif
}

template <class T, class A0>
void test_is_constructible()
{
    static_assert(( std::is_constructible<T, A0>::value), "");
    LIBCPP11_STATIC_ASSERT((std::__libcpp_is_constructible<T, A0>::type::value), "");
#if TEST_STD_VER > 14
    static_assert(( std::is_constructible_v<T, A0>), "");
#endif
}

template <class T, class A0, class A1>
void test_is_constructible()
{
    static_assert(( std::is_constructible<T, A0, A1>::value), "");
    LIBCPP11_STATIC_ASSERT((std::__libcpp_is_constructible<T, A0, A1>::type::value), "");
#if TEST_STD_VER > 14
    static_assert(( std::is_constructible_v<T, A0, A1>), "");
#endif
}

template <class T, class A0, class A1, class A2>
void test_is_constructible()
{
    static_assert(( std::is_constructible<T, A0, A1, A2>::value), "");
    LIBCPP11_STATIC_ASSERT((std::__libcpp_is_constructible<T, A0, A1, A2>::type::value), "");
#if TEST_STD_VER > 14
    static_assert(( std::is_constructible_v<T, A0, A1, A2>), "");
#endif
}

template <class T>
void test_is_not_constructible()
{
    static_assert((!std::is_constructible<T>::value), "");
    LIBCPP11_STATIC_ASSERT((!std::__libcpp_is_constructible<T>::type::value), "");
#if TEST_STD_VER > 14
    static_assert((!std::is_constructible_v<T>), "");
#endif
}

template <class T, class A0>
void test_is_not_constructible()
{
    static_assert((!std::is_constructible<T, A0>::value), "");
    LIBCPP11_STATIC_ASSERT((!std::__libcpp_is_constructible<T, A0>::type::value), "");
#if TEST_STD_VER > 14
    static_assert((!std::is_constructible_v<T, A0>), "");
#endif
}

#if TEST_STD_VER >= 11
template <class T = int, class = decltype(static_cast<T&&>(std::declval<double&>()))>
constexpr bool  clang_disallows_valid_static_cast_test(int) { return false; };

constexpr bool clang_disallows_valid_static_cast_test(long) { return true; }

static constexpr bool clang_disallows_valid_static_cast_bug =
    clang_disallows_valid_static_cast_test(0);
#endif


int main()
{
    typedef Base B;
    typedef Derived D;

    test_is_constructible<int> ();
    test_is_constructible<int, const int> ();
    test_is_constructible<A, int> ();
    test_is_constructible<A, int, double> ();
    test_is_constructible<A, int, long, double> ();
    test_is_constructible<int&, int&> ();

    test_is_not_constructible<A> ();
#if TEST_STD_VER >= 11
    test_is_not_constructible<A, char> ();
#else
    test_is_constructible<A, char> ();
#endif
    test_is_not_constructible<A, void> ();
    test_is_not_constructible<int, void()>();
    test_is_not_constructible<int, void(&)()>();
    test_is_not_constructible<int, void() const>();
    test_is_not_constructible<int&, void>();
    test_is_not_constructible<int&, void()>();
    test_is_not_constructible<int&, void() const>();
    test_is_not_constructible<int&, void(&)()>();

    test_is_not_constructible<void> ();
    test_is_not_constructible<const void> ();  // LWG 2738
    test_is_not_constructible<volatile void> ();
    test_is_not_constructible<const volatile void> ();
    test_is_not_constructible<int&> ();
    test_is_not_constructible<Abstract> ();
    test_is_not_constructible<AbstractDestructor> ();
    test_is_constructible<int, S>();
    test_is_not_constructible<int&, S>();

    test_is_constructible<void(&)(), void(&)()>();
    test_is_constructible<void(&)(), void()>();
#if TEST_STD_VER >= 11
    test_is_constructible<void(&&)(), void(&&)()>();
    test_is_constructible<void(&&)(), void()>();
    test_is_constructible<void(&&)(), void(&)()>();
#endif

#if TEST_STD_VER >= 11
    test_is_constructible<int const&, int>();
    test_is_constructible<int const&, int&&>();

    test_is_constructible<int&&, double&>();
    test_is_constructible<void(&)(), void(&&)()>();

    test_is_not_constructible<int&, int>();
    test_is_not_constructible<int&, int const&>();
    test_is_not_constructible<int&, int&&>();

    test_is_constructible<int&&, int>();
    test_is_constructible<int&&, int&&>();
    test_is_not_constructible<int&&, int&>();
    test_is_not_constructible<int&&, int const&&>();

    test_is_constructible<Base, Derived>();
    test_is_constructible<Base&, Derived&>();
    test_is_not_constructible<Derived&, Base&>();
    test_is_constructible<Base const&, Derived const&>();
    test_is_not_constructible<Derived const&, Base const&>();
    test_is_not_constructible<Derived const&, Base>();

    test_is_constructible<Base&&, Derived>();
    test_is_constructible<Base&&, Derived&&>();
    test_is_not_constructible<Derived&&, Base&&>();
    test_is_not_constructible<Derived&&, Base>();

    // test that T must also be destructible
    test_is_constructible<PrivateDtor&, PrivateDtor&>();
    test_is_not_constructible<PrivateDtor, int>();

    test_is_not_constructible<void() const, void() const>();
    test_is_not_constructible<void() const, void*>();

    test_is_constructible<int&, ImplicitTo<int&>>();
    test_is_constructible<const int&, ImplicitTo<int&&>>();
    test_is_constructible<int&&, ImplicitTo<int&&>>();
    test_is_constructible<const int&, ImplicitTo<int>>();

    test_is_not_constructible<B&&, B&>();
    test_is_not_constructible<B&&, D&>();
    test_is_constructible<B&&, ImplicitTo<D&&>>();
    test_is_constructible<B&&, ImplicitTo<D&&>&>();
    test_is_constructible<int&&, double&>();
    test_is_constructible<const int&, ImplicitTo<int&>&>();
    test_is_constructible<const int&, ImplicitTo<int&>>();
    test_is_constructible<const int&, ExplicitTo<int&>&>();
    test_is_constructible<const int&, ExplicitTo<int&>>();

    test_is_constructible<const int&, ExplicitTo<int&>&>();
    test_is_constructible<const int&, ExplicitTo<int&>>();
    test_is_constructible<int&, ExplicitTo<int&>>();
    test_is_constructible<const int&, ExplicitTo<int&&>>();

    // Binding through reference-compatible type is required to perform
    // direct-initialization as described in [over.match.ref] p. 1 b. 1:
    test_is_constructible<int&, ExplicitTo<int&>>();
    test_is_constructible<const int&, ExplicitTo<int&&>>();

    static_assert(std::is_constructible<int&&, ExplicitTo<int&&>>::value, "");
#ifdef __clang__
#if defined(CLANG_TEST_VER) && CLANG_TEST_VER < 400
    static_assert(clang_disallows_valid_static_cast_bug, "bug still exists");
#endif
    // FIXME Clang disallows this construction because it thinks that
    // 'static_cast<int&&>(declval<ExplicitTo<int&&>>())' is ill-formed.
    LIBCPP_STATIC_ASSERT(
        clang_disallows_valid_static_cast_bug !=
        std::__libcpp_is_constructible<int&&, ExplicitTo<int&&>>::value, "");
    ((void)clang_disallows_valid_static_cast_bug); // Prevent unused warning
#else
    static_assert(clang_disallows_valid_static_cast_bug == false, "");
    LIBCPP_STATIC_ASSERT(std::__libcpp_is_constructible<int&&, ExplicitTo<int&&>>::value, "");
#endif

#ifdef __clang__
    // FIXME Clang and GCC disagree on the validity of this expression.
    test_is_constructible<const int&, ExplicitTo<int>>();
    static_assert(std::is_constructible<int&&, ExplicitTo<int>>::value, "");
    LIBCPP_STATIC_ASSERT(
        clang_disallows_valid_static_cast_bug !=
        std::__libcpp_is_constructible<int&&, ExplicitTo<int>>::value, "");
#else
    test_is_not_constructible<const int&, ExplicitTo<int>>();
    test_is_not_constructible<int&&, ExplicitTo<int>>();
#endif

    // Binding through temporary behaves like copy-initialization,
    // see [dcl.init.ref] p. 5, very last sub-bullet:
    test_is_not_constructible<const int&, ExplicitTo<double&&>>();
    test_is_not_constructible<int&&, ExplicitTo<double&&>>();


// TODO: Remove this workaround once Clang <= 3.7 are no longer used regularly.
// In those compiler versions the __is_constructible builtin gives the wrong
// results for abominable function types.
#if (defined(TEST_APPLE_CLANG_VER) && TEST_APPLE_CLANG_VER < 703) \
 || (defined(TEST_CLANG_VER) && TEST_CLANG_VER < 308)
#define WORKAROUND_CLANG_BUG
#endif
#if !defined(WORKAROUND_CLANG_BUG)
    test_is_not_constructible<void()>();
    test_is_not_constructible<void() const> ();
    test_is_not_constructible<void() volatile> ();
    test_is_not_constructible<void() &> ();
    test_is_not_constructible<void() &&> ();
#endif
#endif // TEST_STD_VER >= 11
}
