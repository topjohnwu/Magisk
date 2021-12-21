//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14

// <tuple>

// template <class F, class T> constexpr decltype(auto) apply(F &&, T &&)

// Testing extended function types. The extended function types are those
// named by INVOKE but that are not actual callable objects. These include
// bullets 1-4 of invoke.

#include <tuple>
#include <array>
#include <utility>
#include <cassert>

// std::array is explicitly allowed to be initialized with A a = { init-list };.
// Disable the missing braces warning for this reason.
#include "disable_missing_braces_warning.h"

int count = 0;

struct A_int_0
{
    A_int_0() : obj1(0){}
    A_int_0(int x) : obj1(x) {}
    int mem1() { return ++count; }
    int mem2() const { return ++count; }
    int const obj1;
};

struct A_int_1
{
    A_int_1() {}
    A_int_1(int) {}
    int mem1(int x) { return count += x; }
    int mem2(int x) const { return count += x; }
};

struct A_int_2
{
    A_int_2() {}
    A_int_2(int) {}
    int mem1(int x, int y) { return count += (x + y); }
    int mem2(int x, int y) const { return count += (x + y); }
};

template <class A>
struct A_wrap
{
    A_wrap() {}
    A_wrap(int x) : m_a(x) {}
    A & operator*() { return m_a; }
    A const & operator*() const { return m_a; }
    A m_a;
};

typedef A_wrap<A_int_0> A_wrap_0;
typedef A_wrap<A_int_1> A_wrap_1;
typedef A_wrap<A_int_2> A_wrap_2;


template <class A>
struct A_base : public A
{
    A_base() : A() {}
    A_base(int x) : A(x) {}
};

typedef A_base<A_int_0> A_base_0;
typedef A_base<A_int_1> A_base_1;
typedef A_base<A_int_2> A_base_2;


template <
    class Tuple, class ConstTuple
  , class TuplePtr, class ConstTuplePtr
  , class TupleWrap, class ConstTupleWrap
  , class TupleBase, class ConstTupleBase
  >
void test_ext_int_0()
{
    count = 0;
    typedef A_int_0 T;
    typedef A_wrap_0 Wrap;
    typedef A_base_0 Base;

    typedef int(T::*mem1_t)();
    mem1_t mem1 = &T::mem1;

    typedef int(T::*mem2_t)() const;
    mem2_t mem2 = &T::mem2;

    typedef int const T::*obj1_t;
    obj1_t obj1 = &T::obj1;

    // member function w/ref
    {
        T a;
        Tuple t{a};
        assert(1 == std::apply(mem1, t));
        assert(count == 1);
    }
    count = 0;
    // member function w/pointer
    {
        T a;
        TuplePtr t{&a};
        assert(1 == std::apply(mem1, t));
        assert(count == 1);
    }
    count = 0;
    // member function w/base
    {
        Base a;
        TupleBase t{a};
        assert(1 == std::apply(mem1, t));
        assert(count == 1);
    }
    count = 0;
    // member function w/wrap
    {
        Wrap a;
        TupleWrap t{a};
        assert(1 == std::apply(mem1, t));
        assert(count == 1);
    }
    count = 0;
    // const member function w/ref
    {
        T const a;
        ConstTuple t{a};
        assert(1 == std::apply(mem2, t));
        assert(count == 1);
    }
    count = 0;
    // const member function w/pointer
    {
        T const a;
        ConstTuplePtr t{&a};
        assert(1 == std::apply(mem2, t));
        assert(count == 1);
    }
    count = 0;
    // const member function w/base
    {
        Base const a;
        ConstTupleBase t{a};
        assert(1 == std::apply(mem2, t));
        assert(count == 1);
    }
    count = 0;
    // const member function w/wrapper
    {
        Wrap const a;
        ConstTupleWrap t{a};
        assert(1 == std::apply(mem2, t));
        assert(1 == count);
    }
    // member object w/ref
    {
        T a{42};
        Tuple t{a};
        assert(42 == std::apply(obj1, t));
    }
    // member object w/pointer
    {
        T a{42};
        TuplePtr t{&a};
        assert(42 == std::apply(obj1, t));
    }
    // member object w/base
    {
        Base a{42};
        TupleBase t{a};
        assert(42 == std::apply(obj1, t));
    }
    // member object w/wrapper
    {
        Wrap a{42};
        TupleWrap t{a};
        assert(42 == std::apply(obj1, t));
    }
}


template <
    class Tuple, class ConstTuple
  , class TuplePtr, class ConstTuplePtr
  , class TupleWrap, class ConstTupleWrap
  , class TupleBase, class ConstTupleBase
  >
void test_ext_int_1()
{
    count = 0;
    typedef A_int_1 T;
    typedef A_wrap_1 Wrap;
    typedef A_base_1 Base;

    typedef int(T::*mem1_t)(int);
    mem1_t mem1 = &T::mem1;

    typedef int(T::*mem2_t)(int) const;
    mem2_t mem2 = &T::mem2;

    // member function w/ref
    {
        T a;
        Tuple t{a, 2};
        assert(2 == std::apply(mem1, t));
        assert(count == 2);
    }
    count = 0;
    // member function w/pointer
    {
        T a;
        TuplePtr t{&a, 3};
        assert(3 == std::apply(mem1, t));
        assert(count == 3);
    }
    count = 0;
    // member function w/base
    {
        Base a;
        TupleBase t{a, 4};
        assert(4 == std::apply(mem1, t));
        assert(count == 4);
    }
    count = 0;
    // member function w/wrap
    {
        Wrap a;
        TupleWrap t{a, 5};
        assert(5 == std::apply(mem1, t));
        assert(count == 5);
    }
    count = 0;
    // const member function w/ref
    {
        T const a;
        ConstTuple t{a, 6};
        assert(6 == std::apply(mem2, t));
        assert(count == 6);
    }
    count = 0;
    // const member function w/pointer
    {
        T const a;
        ConstTuplePtr t{&a, 7};
        assert(7 == std::apply(mem2, t));
        assert(count == 7);
    }
    count = 0;
    // const member function w/base
    {
        Base const a;
        ConstTupleBase t{a, 8};
        assert(8 == std::apply(mem2, t));
        assert(count == 8);
    }
    count = 0;
    // const member function w/wrapper
    {
        Wrap const a;
        ConstTupleWrap t{a, 9};
        assert(9 == std::apply(mem2, t));
        assert(9 == count);
    }
}


template <
    class Tuple, class ConstTuple
  , class TuplePtr, class ConstTuplePtr
  , class TupleWrap, class ConstTupleWrap
  , class TupleBase, class ConstTupleBase
  >
void test_ext_int_2()
{
    count = 0;
    typedef A_int_2 T;
    typedef A_wrap_2 Wrap;
    typedef A_base_2 Base;

    typedef int(T::*mem1_t)(int, int);
    mem1_t mem1 = &T::mem1;

    typedef int(T::*mem2_t)(int, int) const;
    mem2_t mem2 = &T::mem2;

    // member function w/ref
    {
        T a;
        Tuple t{a, 1, 1};
        assert(2 == std::apply(mem1, t));
        assert(count == 2);
    }
    count = 0;
    // member function w/pointer
    {
        T a;
        TuplePtr t{&a, 1, 2};
        assert(3 == std::apply(mem1, t));
        assert(count == 3);
    }
    count = 0;
    // member function w/base
    {
        Base a;
        TupleBase t{a, 2, 2};
        assert(4 == std::apply(mem1, t));
        assert(count == 4);
    }
    count = 0;
    // member function w/wrap
    {
        Wrap a;
        TupleWrap t{a, 2, 3};
        assert(5 == std::apply(mem1, t));
        assert(count == 5);
    }
    count = 0;
    // const member function w/ref
    {
        T const a;
        ConstTuple t{a, 3, 3};
        assert(6 == std::apply(mem2, t));
        assert(count == 6);
    }
    count = 0;
    // const member function w/pointer
    {
        T const a;
        ConstTuplePtr t{&a, 3, 4};
        assert(7 == std::apply(mem2, t));
        assert(count == 7);
    }
    count = 0;
    // const member function w/base
    {
        Base const a;
        ConstTupleBase t{a, 4, 4};
        assert(8 == std::apply(mem2, t));
        assert(count == 8);
    }
    count = 0;
    // const member function w/wrapper
    {
        Wrap const a;
        ConstTupleWrap t{a, 4, 5};
        assert(9 == std::apply(mem2, t));
        assert(9 == count);
    }
}

int main()
{
    {
        test_ext_int_0<
            std::tuple<A_int_0 &>, std::tuple<A_int_0 const &>
          , std::tuple<A_int_0 *>, std::tuple<A_int_0 const *>
          , std::tuple<A_wrap_0 &>, std::tuple<A_wrap_0 const &>
          , std::tuple<A_base_0 &>, std::tuple<A_base_0 const &>
          >();
        test_ext_int_0<
            std::tuple<A_int_0>, std::tuple<A_int_0 const>
          , std::tuple<A_int_0 *>, std::tuple<A_int_0 const *>
          , std::tuple<A_wrap_0>, std::tuple<A_wrap_0 const>
          , std::tuple<A_base_0>, std::tuple<A_base_0 const>
          >();
        test_ext_int_0<
            std::array<A_int_0, 1>, std::array<A_int_0 const, 1>
          , std::array<A_int_0*, 1>, std::array<A_int_0 const*, 1>
          , std::array<A_wrap_0, 1>, std::array<A_wrap_0 const, 1>
          , std::array<A_base_0, 1>, std::array<A_base_0 const, 1>
          >();
    }
    {
        test_ext_int_1<
            std::tuple<A_int_1 &, int>, std::tuple<A_int_1 const &, int>
          , std::tuple<A_int_1 *, int>, std::tuple<A_int_1 const *, int>
          , std::tuple<A_wrap_1 &, int>, std::tuple<A_wrap_1 const &, int>
          , std::tuple<A_base_1 &, int>, std::tuple<A_base_1 const &, int>
          >();
        test_ext_int_1<
            std::tuple<A_int_1, int>, std::tuple<A_int_1 const, int>
          , std::tuple<A_int_1 *, int>, std::tuple<A_int_1 const *, int>
          , std::tuple<A_wrap_1, int>, std::tuple<A_wrap_1 const, int>
          , std::tuple<A_base_1, int>, std::tuple<A_base_1 const, int>
          >();
        test_ext_int_1<
            std::pair<A_int_1 &, int>, std::pair<A_int_1 const &, int>
          , std::pair<A_int_1 *, int>, std::pair<A_int_1 const *, int>
          , std::pair<A_wrap_1 &, int>, std::pair<A_wrap_1 const &, int>
          , std::pair<A_base_1 &, int>, std::pair<A_base_1 const &, int>
          >();
        test_ext_int_1<
            std::pair<A_int_1, int>, std::pair<A_int_1 const, int>
          , std::pair<A_int_1 *, int>, std::pair<A_int_1 const *, int>
          , std::pair<A_wrap_1, int>, std::pair<A_wrap_1 const, int>
          , std::pair<A_base_1, int>, std::pair<A_base_1 const, int>
          >();
    }
    {
        test_ext_int_2<
            std::tuple<A_int_2 &, int, int>, std::tuple<A_int_2 const &, int, int>
          , std::tuple<A_int_2 *, int, int>, std::tuple<A_int_2 const *, int, int>
          , std::tuple<A_wrap_2 &, int, int>, std::tuple<A_wrap_2 const &, int, int>
          , std::tuple<A_base_2 &, int, int>, std::tuple<A_base_2 const &, int, int>
          >();
        test_ext_int_2<
            std::tuple<A_int_2, int, int>, std::tuple<A_int_2 const, int, int>
          , std::tuple<A_int_2 *, int, int>, std::tuple<A_int_2 const *, int, int>
          , std::tuple<A_wrap_2, int, int>, std::tuple<A_wrap_2 const, int, int>
          , std::tuple<A_base_2, int, int>, std::tuple<A_base_2 const, int, int>
          >();
    }
}
