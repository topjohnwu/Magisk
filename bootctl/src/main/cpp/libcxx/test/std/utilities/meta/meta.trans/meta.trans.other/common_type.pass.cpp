//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// common_type

#include <functional>
#include <memory>
#include <type_traits>

#include "test_macros.h"

struct E {};

template <class T>
struct X { explicit X(T const&){} };

template <class T>
struct S { explicit S(T const&){} };

namespace std
{
    template <typename T>
    struct common_type<T, ::S<T> >
    {
        typedef S<T> type;
    };

    template <class T>
    struct common_type< ::S<T>, T> {
      typedef S<T> type;
    };

//  P0548
    template <class T>
    struct common_type< ::S<T>, ::S<T> > {
      typedef S<T> type;
    };

    template <> struct common_type< ::S<long>, long> {};
    template <> struct common_type<long, ::S<long> > {};
    template <> struct common_type< ::X<double>, ::X<double> > {};
}

#if TEST_STD_VER >= 11
template <class Tp>
struct always_bool_imp { using type = bool; };
template <class Tp> using always_bool = typename always_bool_imp<Tp>::type;

template <class ...Args>
constexpr auto no_common_type_imp(int)
  -> always_bool<typename std::common_type<Args...>::type>
  { return false; }

template <class ...Args>
constexpr bool no_common_type_imp(long) { return true; }

template <class ...Args>
using no_common_type = std::integral_constant<bool, no_common_type_imp<Args...>(0)>;

template <class Tp>
using Decay = typename std::decay<Tp>::type;

template <class ...Args>
using CommonType = typename std::common_type<Args...>::type;

template <class T1, class T2>
struct TernaryOpImp {
  static_assert(std::is_same<Decay<T1>, T1>::value, "must be same");
  static_assert(std::is_same<Decay<T2>, T2>::value, "must be same");
  using type = typename std::decay<
      decltype(false ? std::declval<T1>() : std::declval<T2>())
    >::type;
};

template <class T1, class T2>
using TernaryOp = typename TernaryOpImp<T1, T2>::type;

// -- If sizeof...(T) is zero, there shall be no member type.
void test_bullet_one() {
  static_assert(no_common_type<>::value, "");
}

// If sizeof...(T) is one, let T0 denote the sole type constituting the pack T.
// The member typedef-name type shall denote the same type as decay_t<T0>.
void test_bullet_two() {
  static_assert(std::is_same<CommonType<void>, void>::value, "");
  static_assert(std::is_same<CommonType<int>, int>::value, "");
  static_assert(std::is_same<CommonType<int const>, int>::value, "");
  static_assert(std::is_same<CommonType<int volatile[]>, int volatile*>::value, "");
  static_assert(std::is_same<CommonType<void(&)()>, void(*)()>::value, "");

  static_assert(no_common_type<X<double> >::value, "");
}

template <class T, class U, class Expect>
void test_bullet_three_one_imp() {
  using DT = Decay<T>;
  using DU = Decay<U>;
  static_assert(!std::is_same<T, DT>::value || !std::is_same<U, DU>::value, "");
  static_assert(std::is_same<CommonType<T, U>, Expect>::value, "");
  static_assert(std::is_same<CommonType<U, T>, Expect>::value, "");
  static_assert(std::is_same<CommonType<T, U>, CommonType<DT, DU>>::value, "");
}

// (3.3)
// -- If sizeof...(T) is two, let the first and second types constituting T be
//    denoted by T1 and T2, respectively, and let D1 and D2 denote the same types
//    as decay_t<T1> and decay_t<T2>, respectively.
// (3.3.1)
//    -- If is_same_v<T1, D1> is false or is_same_v<T2, D2> is false, let C
//       denote the same type, if any, as common_type_t<D1, D2>.
void test_bullet_three_one() {
  // Test that the user provided specialization of common_type is used after
  // decaying T1.
  {
    using T1 = S<int> const;
    using T2 = int;
    test_bullet_three_one_imp<T1, T2, S<int> >();
  }
  // Test a user provided specialization that does not provide a typedef.
  {
    using T1 = ::S<long> const;
    using T2 = long;
    static_assert(no_common_type<T1, T2>::value, "");
    static_assert(no_common_type<T2, T1>::value, "");
  }
  // Test that the ternary operator is not applied when the types are the
  // same.
  {
    using T1 = const void;
    using Expect = void;
    static_assert(std::is_same<CommonType<T1, T1>, Expect>::value, "");
    static_assert(std::is_same<CommonType<T1, T1>, CommonType<T1>>::value, "");
  }
  {
    using T1 = int const[];
    using Expect = int const*;
    static_assert(std::is_same<CommonType<T1, T1>, Expect>::value, "");
    static_assert(std::is_same<CommonType<T1, T1>, CommonType<T1>>::value, "");
  }
}

// (3.3)
// -- If sizeof...(T) is two, let the first and second types constituting T be
//    denoted by T1 and T2, respectively, and let D1 and D2 denote the same types
//    as decay_t<T1> and decay_t<T2>, respectively.
// (3.3.1)
//    -- If [...]
// (3.3.2)
//    -- Otherwise, let C denote the same type, if any, as
//       decay_t<decltype(false ? declval<D1>() : declval<D2>())>
void test_bullet_three_two() {
  {
    using T1 = int const*;
    using T2 = int*;
    using Expect = TernaryOp<T1, T2>;
    static_assert(std::is_same<CommonType<T1, T2>, Expect>::value, "");
    static_assert(std::is_same<CommonType<T2, T1>, Expect>::value, "");
  }
  // Test that there is no ::type member when the ternary op is ill-formed
  {
    using T1 = int;
    using T2 = void;
    static_assert(no_common_type<T1, T2>::value, "");
    static_assert(no_common_type<T2, T1>::value, "");
  }
  {
    using T1 = int;
    using T2 = X<int>;
    static_assert(no_common_type<T1, T2>::value, "");
    static_assert(no_common_type<T2, T1>::value, "");
  }
  // Test that the ternary operator is not applied when the types are the
  // same.
  {
    using T1 = void;
    using Expect = void;
    static_assert(std::is_same<CommonType<T1, T1>, Expect>::value, "");
    static_assert(std::is_same<CommonType<T1, T1>, CommonType<T1>>::value, "");
  }
}

// (3.4)
// -- If sizeof...(T) is greater than two, let T1, T2, and R, respectively,
// denote the first, second, and (pack of) remaining types constituting T.
// Let C denote the same type, if any, as common_type_t<T1, T2>. If there is
// such a type C, the member typedef-name type shall denote the
// same type, if any, as common_type_t<C, R...>. Otherwise, there shall be
// no member type.
void test_bullet_four() {
  { // test that there is no ::type member
    static_assert(no_common_type<int, E>::value, "");
    static_assert(no_common_type<int, int, E>::value, "");
    static_assert(no_common_type<int, int, E, int>::value, "");
    static_assert(no_common_type<int, int, int, E>::value, "");
  }
}


// The example code specified in Note B for common_type
namespace note_b_example {

using PF1 = bool (&)();
using PF2 = short (*)(long);

struct S {
  operator PF2() const;
  double operator()(char, int&);
  void fn(long) const;
  char data;
};

using PMF = void (S::*)(long) const;
using PMD = char S::*;

using std::is_same;
using std::result_of;
using std::unique_ptr;

static_assert(is_same<typename result_of<S(int)>::type, short>::value, "Error!");
static_assert(is_same<typename result_of<S&(unsigned char, int&)>::type, double>::value, "Error!");
static_assert(is_same<typename result_of<PF1()>::type, bool>::value, "Error!");
static_assert(is_same<typename result_of<PMF(unique_ptr<S>, int)>::type, void>::value, "Error!");
static_assert(is_same<typename result_of<PMD(S)>::type, char&&>::value, "Error!");
static_assert(is_same<typename result_of<PMD(const S*)>::type, const char&>::value, "Error!");

} // namespace note_b_example
#endif // TEST_STD_VER >= 11

int main()
{
    static_assert((std::is_same<std::common_type<int>::type, int>::value), "");
    static_assert((std::is_same<std::common_type<char>::type, char>::value), "");
#if TEST_STD_VER > 11
    static_assert((std::is_same<std::common_type_t<int>,   int>::value), "");
    static_assert((std::is_same<std::common_type_t<char>, char>::value), "");
#endif

    static_assert((std::is_same<std::common_type<               int>::type, int>::value), "");
    static_assert((std::is_same<std::common_type<const          int>::type, int>::value), "");
    static_assert((std::is_same<std::common_type<      volatile int>::type, int>::value), "");
    static_assert((std::is_same<std::common_type<const volatile int>::type, int>::value), "");

    static_assert((std::is_same<std::common_type<int,           int>::type, int>::value), "");
    static_assert((std::is_same<std::common_type<int,     const int>::type, int>::value), "");

    static_assert((std::is_same<std::common_type<long,       const int>::type, long>::value), "");
    static_assert((std::is_same<std::common_type<const long,       int>::type, long>::value), "");
    static_assert((std::is_same<std::common_type<long,    volatile int>::type, long>::value), "");
    static_assert((std::is_same<std::common_type<volatile long,    int>::type, long>::value), "");
    static_assert((std::is_same<std::common_type<const long, const int>::type, long>::value), "");

    static_assert((std::is_same<std::common_type<double, char>::type, double>::value), "");
    static_assert((std::is_same<std::common_type<short, char>::type, int>::value), "");
#if TEST_STD_VER > 11
    static_assert((std::is_same<std::common_type_t<double, char>, double>::value), "");
    static_assert((std::is_same<std::common_type_t<short, char>, int>::value), "");
#endif

    static_assert((std::is_same<std::common_type<double, char, long long>::type, double>::value), "");
    static_assert((std::is_same<std::common_type<unsigned, char, long long>::type, long long>::value), "");
#if TEST_STD_VER > 11
    static_assert((std::is_same<std::common_type_t<double, char, long long>, double>::value), "");
    static_assert((std::is_same<std::common_type_t<unsigned, char, long long>, long long>::value), "");
#endif

    static_assert((std::is_same<std::common_type<               void>::type, void>::value), "");
    static_assert((std::is_same<std::common_type<const          void>::type, void>::value), "");
    static_assert((std::is_same<std::common_type<      volatile void>::type, void>::value), "");
    static_assert((std::is_same<std::common_type<const volatile void>::type, void>::value), "");

    static_assert((std::is_same<std::common_type<void,       const void>::type, void>::value), "");
    static_assert((std::is_same<std::common_type<const void,       void>::type, void>::value), "");
    static_assert((std::is_same<std::common_type<void,    volatile void>::type, void>::value), "");
    static_assert((std::is_same<std::common_type<volatile void,    void>::type, void>::value), "");
    static_assert((std::is_same<std::common_type<const void, const void>::type, void>::value), "");

    static_assert((std::is_same<std::common_type<int, S<int> >::type, S<int> >::value), "");
    static_assert((std::is_same<std::common_type<int, S<int>, S<int> >::type, S<int> >::value), "");
    static_assert((std::is_same<std::common_type<int, int, S<int> >::type, S<int> >::value), "");

#if TEST_STD_VER >= 11
  test_bullet_one();
  test_bullet_two();
  test_bullet_three_one();
  test_bullet_three_two();
  test_bullet_four();
#endif

//  P0548
    static_assert((std::is_same<std::common_type<S<int> >::type,         S<int> >::value), "");
    static_assert((std::is_same<std::common_type<S<int>, S<int> >::type, S<int> >::value), "");

    static_assert((std::is_same<std::common_type<int>::type,                int>::value), "");
    static_assert((std::is_same<std::common_type<const int>::type,          int>::value), "");
    static_assert((std::is_same<std::common_type<volatile int>::type,       int>::value), "");
    static_assert((std::is_same<std::common_type<const volatile int>::type, int>::value), "");

    static_assert((std::is_same<std::common_type<int, int>::type,             int>::value), "");
    static_assert((std::is_same<std::common_type<const int, int>::type,       int>::value), "");
    static_assert((std::is_same<std::common_type<int, const int>::type,       int>::value), "");
    static_assert((std::is_same<std::common_type<const int, const int>::type, int>::value), "");
}
