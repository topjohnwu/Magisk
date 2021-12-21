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

// template <class T, class Tuple> constexpr T make_from_tuple(Tuple&&);

#include <tuple>
#include <array>
#include <utility>
#include <string>
#include <cassert>

#include "test_macros.h"
#include "type_id.h"

// std::array is explicitly allowed to be initialized with A a = { init-list };.
// Disable the missing braces warning for this reason.
#include "disable_missing_braces_warning.h"

template <class Tuple>
struct ConstexprConstructibleFromTuple {
  template <class ...Args>
  explicit constexpr ConstexprConstructibleFromTuple(Args&&... xargs)
      : args{std::forward<Args>(xargs)...} {}
  Tuple args;
};

template <class TupleLike>
struct ConstructibleFromTuple;

template <template <class ...> class Tuple, class ...Types>
struct ConstructibleFromTuple<Tuple<Types...>> {
  template <class ...Args>
  explicit ConstructibleFromTuple(Args&&... xargs)
      : args(xargs...),
        arg_types(&makeArgumentID<Args&&...>())
  {}
  Tuple<std::decay_t<Types>...> args;
  TypeID const* arg_types;
};

template <class Tp, size_t N>
struct ConstructibleFromTuple<std::array<Tp, N>> {
template <class ...Args>
  explicit ConstructibleFromTuple(Args&&... xargs)
      : args{xargs...},
        arg_types(&makeArgumentID<Args&&...>())
  {}
  std::array<Tp, N> args;
  TypeID const* arg_types;
};

template <class Tuple>
constexpr bool do_constexpr_test(Tuple&& tup) {
    using RawTuple = std::decay_t<Tuple>;
    using Tp = ConstexprConstructibleFromTuple<RawTuple>;
    return std::make_from_tuple<Tp>(std::forward<Tuple>(tup)).args == tup;
}

// Needed by do_forwarding_test() since it compares pairs of different types.
template <class T1, class T2, class U1, class U2>
inline bool operator==(const std::pair<T1, T2>& lhs, const std::pair<U1, U2>& rhs) {
    return lhs.first == rhs.first && lhs.second == rhs.second;
}

template <class ...ExpectTypes, class Tuple>
bool do_forwarding_test(Tuple&& tup) {
    using RawTuple = std::decay_t<Tuple>;
    using Tp = ConstructibleFromTuple<RawTuple>;
    const Tp value = std::make_from_tuple<Tp>(std::forward<Tuple>(tup));
    return value.args == tup
        && value.arg_types == &makeArgumentID<ExpectTypes...>();
}

void test_constexpr_construction() {
    {
        constexpr std::tuple<> tup;
        static_assert(do_constexpr_test(tup), "");
    }
    {
        constexpr std::tuple<int> tup(42);
        static_assert(do_constexpr_test(tup), "");
    }
    {
        constexpr std::tuple<int, long, void*> tup(42, 101, nullptr);
        static_assert(do_constexpr_test(tup), "");
    }
    {
        constexpr std::pair<int, const char*> p(42, "hello world");
        static_assert(do_constexpr_test(p), "");
    }
    {
        using Tuple = std::array<int, 3>;
        using ValueTp = ConstexprConstructibleFromTuple<Tuple>;
        constexpr Tuple arr = {42, 101, -1};
        constexpr ValueTp value = std::make_from_tuple<ValueTp>(arr);
        static_assert(value.args[0] == arr[0] && value.args[1] == arr[1]
            && value.args[2] == arr[2], "");
    }
}

void test_perfect_forwarding() {
    {
        using Tup = std::tuple<>;
        Tup tup;
        Tup const& ctup = tup;
        assert(do_forwarding_test<>(tup));
        assert(do_forwarding_test<>(ctup));
    }
    {
        using Tup = std::tuple<int>;
        Tup tup(42);
        Tup const& ctup = tup;
        assert(do_forwarding_test<int&>(tup));
        assert(do_forwarding_test<int const&>(ctup));
        assert(do_forwarding_test<int&&>(std::move(tup)));
        assert(do_forwarding_test<int const&&>(std::move(ctup)));
    }
    {
        using Tup = std::tuple<int&, const char*, unsigned&&>;
        int x = 42;
        unsigned y = 101;
        Tup tup(x, "hello world", std::move(y));
        Tup const& ctup = tup;
        assert((do_forwarding_test<int&, const char*&, unsigned&>(tup)));
        assert((do_forwarding_test<int&, const char* const&, unsigned &>(ctup)));
        assert((do_forwarding_test<int&, const char*&&, unsigned&&>(std::move(tup))));
        assert((do_forwarding_test<int&, const char* const&&, unsigned &&>(std::move(ctup))));
    }
    // test with pair<T, U>
    {
        using Tup = std::pair<int&, const char*>;
        int x = 42;
        Tup tup(x, "hello world");
        Tup const& ctup = tup;
        assert((do_forwarding_test<int&, const char*&>(tup)));
        assert((do_forwarding_test<int&, const char* const&>(ctup)));
        assert((do_forwarding_test<int&, const char*&&>(std::move(tup))));
        assert((do_forwarding_test<int&, const char* const&&>(std::move(ctup))));
    }
    // test with array<T, I>
    {
        using Tup = std::array<int, 3>;
        Tup tup = {42, 101, -1};
        Tup const& ctup = tup;
        assert((do_forwarding_test<int&, int&, int&>(tup)));
        assert((do_forwarding_test<int const&, int const&, int const&>(ctup)));
        assert((do_forwarding_test<int&&, int&&, int&&>(std::move(tup))));
        assert((do_forwarding_test<int const&&, int const&&, int const&&>(std::move(ctup))));
    }
}

void test_noexcept() {
    struct NothrowMoveable {
      NothrowMoveable() = default;
      NothrowMoveable(NothrowMoveable const&) {}
      NothrowMoveable(NothrowMoveable&&) noexcept {}
    };
    struct TestType {
      TestType(int, NothrowMoveable) noexcept {}
      TestType(int, int, int) noexcept(false) {}
      TestType(long, long, long) noexcept {}
    };
    {
        using Tuple = std::tuple<int, NothrowMoveable>;
        Tuple tup; ((void)tup);
        Tuple const& ctup = tup; ((void)ctup);
        ASSERT_NOT_NOEXCEPT(std::make_from_tuple<TestType>(ctup));
        LIBCPP_ASSERT_NOEXCEPT(std::make_from_tuple<TestType>(std::move(tup)));
    }
    {
        using Tuple = std::pair<int, NothrowMoveable>;
        Tuple tup; ((void)tup);
        Tuple const& ctup = tup; ((void)ctup);
        ASSERT_NOT_NOEXCEPT(std::make_from_tuple<TestType>(ctup));
        LIBCPP_ASSERT_NOEXCEPT(std::make_from_tuple<TestType>(std::move(tup)));
    }
    {
        using Tuple = std::tuple<int, int, int>;
        Tuple tup; ((void)tup);
        ASSERT_NOT_NOEXCEPT(std::make_from_tuple<TestType>(tup));
    }
    {
        using Tuple = std::tuple<long, long, long>;
        Tuple tup; ((void)tup);
        LIBCPP_ASSERT_NOEXCEPT(std::make_from_tuple<TestType>(tup));
    }
    {
        using Tuple = std::array<int, 3>;
        Tuple tup; ((void)tup);
        ASSERT_NOT_NOEXCEPT(std::make_from_tuple<TestType>(tup));
    }
    {
        using Tuple = std::array<long, 3>;
        Tuple tup; ((void)tup);
        LIBCPP_ASSERT_NOEXCEPT(std::make_from_tuple<TestType>(tup));
    }
}

int main()
{
    test_constexpr_construction();
    test_perfect_forwarding();
    test_noexcept();
}
