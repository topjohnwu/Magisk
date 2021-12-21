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

// Test with different ref/ptr/cv qualified argument types.

#include <tuple>
#include <array>
#include <utility>
#include <cassert>

#include "test_macros.h"
#include "type_id.h"

// std::array is explicitly allowed to be initialized with A a = { init-list };.
// Disable the missing braces warning for this reason.
#include "disable_missing_braces_warning.h"


constexpr int constexpr_sum_fn() { return 0; }

template <class ...Ints>
constexpr int constexpr_sum_fn(int x1, Ints... rest) { return x1 + constexpr_sum_fn(rest...); }

struct ConstexprSumT {
  constexpr ConstexprSumT() = default;
  template <class ...Ints>
  constexpr int operator()(Ints... values) const {
      return constexpr_sum_fn(values...);
  }
};


void test_constexpr_evaluation()
{
    constexpr ConstexprSumT sum_obj{};
    {
        using Tup = std::tuple<>;
        using Fn = int(&)();
        constexpr Tup t;
        static_assert(std::apply(static_cast<Fn>(constexpr_sum_fn), t) == 0, "");
        static_assert(std::apply(sum_obj, t) == 0, "");
    }
    {
        using Tup = std::tuple<int>;
        using Fn = int(&)(int);
        constexpr Tup t(42);
        static_assert(std::apply(static_cast<Fn>(constexpr_sum_fn), t) == 42, "");
        static_assert(std::apply(sum_obj, t) == 42, "");
    }
    {
        using Tup = std::tuple<int, long>;
        using Fn = int(&)(int, int);
        constexpr Tup t(42, 101);
        static_assert(std::apply(static_cast<Fn>(constexpr_sum_fn), t) == 143, "");
        static_assert(std::apply(sum_obj, t) == 143, "");
    }
    {
        using Tup = std::pair<int, long>;
        using Fn = int(&)(int, int);
        constexpr Tup t(42, 101);
        static_assert(std::apply(static_cast<Fn>(constexpr_sum_fn), t) == 143, "");
        static_assert(std::apply(sum_obj, t) == 143, "");
    }
    {
        using Tup = std::tuple<int, long, int>;
        using Fn = int(&)(int, int, int);
        constexpr Tup t(42, 101, -1);
        static_assert(std::apply(static_cast<Fn>(constexpr_sum_fn), t) == 142, "");
        static_assert(std::apply(sum_obj, t) == 142, "");
    }
    {
        using Tup = std::array<int, 3>;
        using Fn = int(&)(int, int, int);
        constexpr Tup t = {42, 101, -1};
        static_assert(std::apply(static_cast<Fn>(constexpr_sum_fn), t) == 142, "");
        static_assert(std::apply(sum_obj, t) == 142, "");
    }
}


enum CallQuals {
  CQ_None,
  CQ_LValue,
  CQ_ConstLValue,
  CQ_RValue,
  CQ_ConstRValue
};

template <class Tuple>
struct CallInfo {
  CallQuals quals;
  TypeID const* arg_types;
  Tuple args;

  template <class ...Args>
  CallInfo(CallQuals q, Args&&... xargs)
      : quals(q), arg_types(&makeArgumentID<Args&&...>()), args(std::forward<Args>(xargs)...)
  {}
};

template <class ...Args>
inline CallInfo<decltype(std::forward_as_tuple(std::declval<Args>()...))>
makeCallInfo(CallQuals quals, Args&&... args) {
    return {quals, std::forward<Args>(args)...};
}

struct TrackedCallable {

  TrackedCallable() = default;

  template <class ...Args> auto operator()(Args&&... xargs) &
  { return makeCallInfo(CQ_LValue, std::forward<Args>(xargs)...); }

  template <class ...Args> auto operator()(Args&&... xargs) const&
  { return makeCallInfo(CQ_ConstLValue, std::forward<Args>(xargs)...); }

  template <class ...Args> auto operator()(Args&&... xargs) &&
  { return makeCallInfo(CQ_RValue, std::forward<Args>(xargs)...); }

  template <class ...Args> auto operator()(Args&&... xargs) const&&
  { return makeCallInfo(CQ_ConstRValue, std::forward<Args>(xargs)...); }
};

template <class ...ExpectArgs, class Tuple>
void check_apply_quals_and_types(Tuple&& t) {
    TypeID const* const expect_args = &makeArgumentID<ExpectArgs...>();
    TrackedCallable obj;
    TrackedCallable const& cobj = obj;
    {
        auto ret = std::apply(obj, std::forward<Tuple>(t));
        assert(ret.quals == CQ_LValue);
        assert(ret.arg_types == expect_args);
        assert(ret.args == t);
    }
    {
        auto ret = std::apply(cobj, std::forward<Tuple>(t));
        assert(ret.quals == CQ_ConstLValue);
        assert(ret.arg_types == expect_args);
        assert(ret.args == t);
    }
    {
        auto ret = std::apply(std::move(obj), std::forward<Tuple>(t));
        assert(ret.quals == CQ_RValue);
        assert(ret.arg_types == expect_args);
        assert(ret.args == t);
    }
    {
        auto ret = std::apply(std::move(cobj), std::forward<Tuple>(t));
        assert(ret.quals == CQ_ConstRValue);
        assert(ret.arg_types == expect_args);
        assert(ret.args == t);
    }
}

void test_call_quals_and_arg_types()
{
    using Tup = std::tuple<int, int const&, unsigned&&>;
    const int x = 42;
    unsigned y = 101;
    Tup t(-1, x, std::move(y));
    Tup const& ct = t;
    check_apply_quals_and_types<int&, int const&, unsigned&>(t);
    check_apply_quals_and_types<int const&, int const&, unsigned&>(ct);
    check_apply_quals_and_types<int&&, int const&, unsigned&&>(std::move(t));
    check_apply_quals_and_types<int const&&, int const&, unsigned&&>(std::move(ct));
}


struct NothrowMoveable {
  NothrowMoveable() noexcept = default;
  NothrowMoveable(NothrowMoveable const&) noexcept(false) {}
  NothrowMoveable(NothrowMoveable&&) noexcept {}
};

template <bool IsNoexcept>
struct TestNoexceptCallable {
  template <class ...Args>
  NothrowMoveable operator()(Args...) const noexcept(IsNoexcept) { return {}; }
};

void test_noexcept()
{
    TestNoexceptCallable<true> nec;
    TestNoexceptCallable<false> tc;
    {
        // test that the functions noexcept-ness is propagated
        using Tup = std::tuple<int, const char*, long>;
        Tup t;
        LIBCPP_ASSERT_NOEXCEPT(std::apply(nec, t));
        ASSERT_NOT_NOEXCEPT(std::apply(tc, t));
    }
    {
        // test that the noexcept-ness of the argument conversions is checked.
        using Tup = std::tuple<NothrowMoveable, int>;
        Tup t;
        ASSERT_NOT_NOEXCEPT(std::apply(nec, t));
        LIBCPP_ASSERT_NOEXCEPT(std::apply(nec, std::move(t)));
    }
}

namespace ReturnTypeTest {
    static int my_int = 42;

    template <int N> struct index {};

    void f(index<0>) {}

    int f(index<1>) { return 0; }

    int & f(index<2>) { return static_cast<int &>(my_int); }
    int const & f(index<3>) { return static_cast<int const &>(my_int); }
    int volatile & f(index<4>) { return static_cast<int volatile &>(my_int); }
    int const volatile & f(index<5>) { return static_cast<int const volatile &>(my_int); }

    int && f(index<6>) { return static_cast<int &&>(my_int); }
    int const && f(index<7>) { return static_cast<int const &&>(my_int); }
    int volatile && f(index<8>) { return static_cast<int volatile &&>(my_int); }
    int const volatile && f(index<9>) { return static_cast<int const volatile &&>(my_int); }

    int * f(index<10>) { return static_cast<int *>(&my_int); }
    int const * f(index<11>) { return static_cast<int const *>(&my_int); }
    int volatile * f(index<12>) { return static_cast<int volatile *>(&my_int); }
    int const volatile * f(index<13>) { return static_cast<int const volatile *>(&my_int); }

    template <int Func, class Expect>
    void test()
    {
        using RawInvokeResult = decltype(f(index<Func>{}));
        static_assert(std::is_same<RawInvokeResult, Expect>::value, "");
        using FnType = RawInvokeResult (*) (index<Func>);
        FnType fn = f;
        std::tuple<index<Func>> t; ((void)t);
        using InvokeResult = decltype(std::apply(fn, t));
        static_assert(std::is_same<InvokeResult, Expect>::value, "");
    }
} // end namespace ReturnTypeTest

void test_return_type()
{
    using ReturnTypeTest::test;
    test<0, void>();
    test<1, int>();
    test<2, int &>();
    test<3, int const &>();
    test<4, int volatile &>();
    test<5, int const volatile &>();
    test<6, int &&>();
    test<7, int const &&>();
    test<8, int volatile &&>();
    test<9, int const volatile &&>();
    test<10, int *>();
    test<11, int const *>();
    test<12, int volatile *>();
    test<13, int const volatile *>();
}

int main() {
    test_constexpr_evaluation();
    test_call_quals_and_arg_types();
    test_return_type();
    test_noexcept();
}
