//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14

// type_traits

// is_nothrow_invocable

#include <type_traits>
#include <functional>

#include "test_macros.h"

struct Tag {};

struct Implicit {
  Implicit(int) noexcept {}
};

struct ThrowsImplicit {
  ThrowsImplicit(int) {}
};

struct Explicit {
  explicit Explicit(int) noexcept {}
};

template <bool IsNoexcept, class Ret, class ...Args>
struct CallObject {
  Ret operator()(Args&&...) const noexcept(IsNoexcept);
};

template <class Fn, class ...Args>
constexpr bool throws_invocable() {
    return std::is_invocable<Fn, Args...>::value &&
        !std::is_nothrow_invocable<Fn, Args...>::value;
}

template <class Ret, class Fn, class ...Args>
constexpr bool throws_invocable_r() {
    return std::is_invocable_r<Ret, Fn, Args...>::value &&
        !std::is_nothrow_invocable_r<Ret, Fn, Args...>::value;
}

// FIXME(EricWF) Don't test the where noexcept is *not* part of the type system
// once implementations have caught up.
void test_noexcept_function_pointers()
{
    struct Dummy { void foo() noexcept {} static void bar() noexcept {} };
#if !defined(__cpp_noexcept_function_type)
    {
        // Check that PMF's and function pointers *work*. is_nothrow_invocable will always
        // return false because 'noexcept' is not part of the function type.
        static_assert(throws_invocable<decltype(&Dummy::foo), Dummy&>(), "");
        static_assert(throws_invocable<decltype(&Dummy::bar)>(), "");
    }
#else
    {
        // Check that PMF's and function pointers actually work and that
        // is_nothrow_invocable returns true for noexcept PMF's and function
        // pointers.
        static_assert(std::is_nothrow_invocable<decltype(&Dummy::foo), Dummy&>::value, "");
        static_assert(std::is_nothrow_invocable<decltype(&Dummy::bar)>::value, "");
    }
#endif
}

int main()
{
    {
        // Check that the conversion to the return type is properly checked
        using Fn = CallObject<true, int>;
        static_assert(std::is_nothrow_invocable_r<Implicit, Fn>::value, "");
        static_assert(std::is_nothrow_invocable_r<double, Fn>::value, "");
        static_assert(std::is_nothrow_invocable_r<const volatile void, Fn>::value, "");
        static_assert(throws_invocable_r<ThrowsImplicit, Fn>(), "");
        static_assert(!std::is_nothrow_invocable<Fn(), Explicit>(), "");
    }
    {
        // Check that the conversion to the parameters is properly checked
        using Fn = CallObject<true, void, const Implicit&, const ThrowsImplicit&>;
        static_assert(std::is_nothrow_invocable<Fn, Implicit&, ThrowsImplicit&>::value, "");
        static_assert(std::is_nothrow_invocable<Fn, int, ThrowsImplicit&>::value, "");
        static_assert(throws_invocable<Fn, int, int>(), "");
        static_assert(!std::is_nothrow_invocable<Fn>::value, "");
    }
    {
        // Check that the noexcept-ness of function objects is checked.
        using Fn = CallObject<true, void>;
        using Fn2 = CallObject<false, void>;
        static_assert(std::is_nothrow_invocable<Fn>::value, "");
        static_assert(throws_invocable<Fn2>(), "");
    }
    {
        // Check that PMD derefs are noexcept
        using Fn = int (Tag::*);
        static_assert(std::is_nothrow_invocable<Fn, Tag&>::value, "");
        static_assert(std::is_nothrow_invocable_r<Implicit, Fn, Tag&>::value, "");
        static_assert(throws_invocable_r<ThrowsImplicit, Fn, Tag&>(), "");
    }
    {
        // Check for is_nothrow_invocable_v
        using Fn = CallObject<true, int>;
        static_assert(std::is_nothrow_invocable_v<Fn>, "");
        static_assert(!std::is_nothrow_invocable_v<Fn, int>, "");
    }
    {
        // Check for is_nothrow_invocable_r_v
        using Fn = CallObject<true, int>;
        static_assert(std::is_nothrow_invocable_r_v<void, Fn>, "");
        static_assert(!std::is_nothrow_invocable_r_v<int, Fn, int>, "");
    }
    test_noexcept_function_pointers();
}
