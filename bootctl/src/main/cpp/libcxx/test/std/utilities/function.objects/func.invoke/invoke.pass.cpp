//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14

// <functional>

// template <class F, class ...Args>
// result_of_t<F&&(Args&&...)> invoke(F&&, Args&&...);

/// C++14 [func.def] 20.9.0
/// (1) The following definitions apply to this Clause:
/// (2) A call signature is the name of a return type followed by a parenthesized
///     comma-separated list of zero or more argument types.
/// (3) A callable type is a function object type (20.9) or a pointer to member.
/// (4) A callable object is an object of a callable type.
/// (5) A call wrapper type is a type that holds a callable object and supports
///     a call operation that forwards to that object.
/// (6) A call wrapper is an object of a call wrapper type.
/// (7) A target object is the callable object held by a call wrapper.

/// C++14 [func.require] 20.9.1
///
/// Define INVOKE (f, t1, t2, ..., tN) as follows:
///   (1.1) - (t1.*f)(t2, ..., tN) when f is a pointer to a member function of a class T and t1 is an object of
///   type T or a reference to an object of type T or a reference to an object of a type derived from T;
///   (1.2) - ((*t1).*f)(t2, ..., tN) when f is a pointer to a member function of a class T and t1 is not one of
///   the types described in the previous item;
///   (1.3) - t1.*f when N == 1 and f is a pointer to member data of a class T and t1 is an object of type T or a
///   reference to an object of type T or a reference to an object of a type derived from T;
///   (1.4) - (*t1).*f when N == 1 and f is a pointer to member data of a class T and t1 is not one of the types
///   described in the previous item;
///   (1.5) - f(t1, t2, ..., tN) in all other cases.

#include <functional>
#include <type_traits>
#include <utility> // for std::move
#include <cassert>

struct NonCopyable {
    NonCopyable() {}
private:
    NonCopyable(NonCopyable const&) = delete;
    NonCopyable& operator=(NonCopyable const&) = delete;
};

struct TestClass {
    explicit TestClass(int x) : data(x) {}

    int& operator()(NonCopyable&&) & { return data; }
    int const& operator()(NonCopyable&&) const & { return data; }
    int volatile& operator()(NonCopyable&&) volatile & { return data; }
    int const volatile& operator()(NonCopyable&&) const volatile & { return data; }

    int&& operator()(NonCopyable&&) && { return std::move(data); }
    int const&& operator()(NonCopyable&&) const && { return std::move(data); }
    int volatile&& operator()(NonCopyable&&) volatile && { return std::move(data); }
    int const volatile&& operator()(NonCopyable&&) const volatile && { return std::move(data); }

    int data;
private:
    TestClass(TestClass const&) = delete;
    TestClass& operator=(TestClass const&) = delete;
};

struct DerivedFromTestClass : public TestClass {
    explicit DerivedFromTestClass(int x) : TestClass(x) {}
};

int& foo(NonCopyable&&) {
    static int data = 42;
    return data;
}

template <class Signature,  class Expect, class Functor>
void test_b12(Functor&& f) {
    // Create the callable object.
    typedef Signature TestClass::*ClassFunc;
    ClassFunc func_ptr = &TestClass::operator();

    // Create the dummy arg.
    NonCopyable arg;

    // Check that the deduced return type of invoke is what is expected.
    typedef decltype(
        std::invoke(func_ptr, std::forward<Functor>(f), std::move(arg))
    ) DeducedReturnType;
    static_assert((std::is_same<DeducedReturnType, Expect>::value), "");

    // Check that result_of_t matches Expect.
    typedef typename std::result_of<ClassFunc&&(Functor&&, NonCopyable&&)>::type
      ResultOfReturnType;
    static_assert((std::is_same<ResultOfReturnType, Expect>::value), "");

    // Run invoke and check the return value.
    DeducedReturnType ret =
            std::invoke(func_ptr, std::forward<Functor>(f), std::move(arg));
    assert(ret == 42);
}

template <class Expect, class Functor>
void test_b34(Functor&& f) {
    // Create the callable object.
    typedef int TestClass::*ClassFunc;
    ClassFunc func_ptr = &TestClass::data;

    // Check that the deduced return type of invoke is what is expected.
    typedef decltype(
        std::invoke(func_ptr, std::forward<Functor>(f))
    ) DeducedReturnType;
    static_assert((std::is_same<DeducedReturnType, Expect>::value), "");

    // Check that result_of_t matches Expect.
    typedef typename std::result_of<ClassFunc&&(Functor&&)>::type
            ResultOfReturnType;
    static_assert((std::is_same<ResultOfReturnType, Expect>::value), "");

    // Run invoke and check the return value.
    DeducedReturnType ret =
            std::invoke(func_ptr, std::forward<Functor>(f));
    assert(ret == 42);
}

template <class Expect, class Functor>
void test_b5(Functor&& f) {
    NonCopyable arg;

    // Check that the deduced return type of invoke is what is expected.
    typedef decltype(
        std::invoke(std::forward<Functor>(f), std::move(arg))
    ) DeducedReturnType;
    static_assert((std::is_same<DeducedReturnType, Expect>::value), "");

    // Check that result_of_t matches Expect.
    typedef typename std::result_of<Functor&&(NonCopyable&&)>::type
            ResultOfReturnType;
    static_assert((std::is_same<ResultOfReturnType, Expect>::value), "");

    // Run invoke and check the return value.
    DeducedReturnType ret = std::invoke(std::forward<Functor>(f), std::move(arg));
    assert(ret == 42);
}

void bullet_one_two_tests() {
    {
        TestClass cl(42);
        test_b12<int&(NonCopyable&&) &, int&>(cl);
        test_b12<int const&(NonCopyable&&) const &, int const&>(cl);
        test_b12<int volatile&(NonCopyable&&) volatile &, int volatile&>(cl);
        test_b12<int const volatile&(NonCopyable&&) const volatile &, int const volatile&>(cl);

        test_b12<int&&(NonCopyable&&) &&, int&&>(std::move(cl));
        test_b12<int const&&(NonCopyable&&) const &&, int const&&>(std::move(cl));
        test_b12<int volatile&&(NonCopyable&&) volatile &&, int volatile&&>(std::move(cl));
        test_b12<int const volatile&&(NonCopyable&&) const volatile &&, int const volatile&&>(std::move(cl));
    }
    {
        DerivedFromTestClass cl(42);
        test_b12<int&(NonCopyable&&) &, int&>(cl);
        test_b12<int const&(NonCopyable&&) const &, int const&>(cl);
        test_b12<int volatile&(NonCopyable&&) volatile &, int volatile&>(cl);
        test_b12<int const volatile&(NonCopyable&&) const volatile &, int const volatile&>(cl);

        test_b12<int&&(NonCopyable&&) &&, int&&>(std::move(cl));
        test_b12<int const&&(NonCopyable&&) const &&, int const&&>(std::move(cl));
        test_b12<int volatile&&(NonCopyable&&) volatile &&, int volatile&&>(std::move(cl));
        test_b12<int const volatile&&(NonCopyable&&) const volatile &&, int const volatile&&>(std::move(cl));
    }
    {
        TestClass cl_obj(42);
        std::reference_wrapper<TestClass> cl(cl_obj);
        test_b12<int&(NonCopyable&&) &, int&>(cl);
        test_b12<int const&(NonCopyable&&) const &, int const&>(cl);
        test_b12<int volatile&(NonCopyable&&) volatile &, int volatile&>(cl);
        test_b12<int const volatile&(NonCopyable&&) const volatile &, int const volatile&>(cl);

        test_b12<int&(NonCopyable&&) &, int&>(std::move(cl));
        test_b12<int const&(NonCopyable&&) const &, int const&>(std::move(cl));
        test_b12<int volatile&(NonCopyable&&) volatile &, int volatile&>(std::move(cl));
        test_b12<int const volatile&(NonCopyable&&) const volatile &, int const volatile&>(std::move(cl));
    }
    {
        DerivedFromTestClass cl_obj(42);
        std::reference_wrapper<DerivedFromTestClass> cl(cl_obj);
        test_b12<int&(NonCopyable&&) &, int&>(cl);
        test_b12<int const&(NonCopyable&&) const &, int const&>(cl);
        test_b12<int volatile&(NonCopyable&&) volatile &, int volatile&>(cl);
        test_b12<int const volatile&(NonCopyable&&) const volatile &, int const volatile&>(cl);

        test_b12<int&(NonCopyable&&) &, int&>(std::move(cl));
        test_b12<int const&(NonCopyable&&) const &, int const&>(std::move(cl));
        test_b12<int volatile&(NonCopyable&&) volatile &, int volatile&>(std::move(cl));
        test_b12<int const volatile&(NonCopyable&&) const volatile &, int const volatile&>(std::move(cl));
    }
    {
        TestClass cl_obj(42);
        TestClass *cl = &cl_obj;
        test_b12<int&(NonCopyable&&) &, int&>(cl);
        test_b12<int const&(NonCopyable&&) const &, int const&>(cl);
        test_b12<int volatile&(NonCopyable&&) volatile &, int volatile&>(cl);
        test_b12<int const volatile&(NonCopyable&&) const volatile &, int const volatile&>(cl);
    }
    {
        DerivedFromTestClass cl_obj(42);
        DerivedFromTestClass *cl = &cl_obj;
        test_b12<int&(NonCopyable&&) &, int&>(cl);
        test_b12<int const&(NonCopyable&&) const &, int const&>(cl);
        test_b12<int volatile&(NonCopyable&&) volatile &, int volatile&>(cl);
        test_b12<int const volatile&(NonCopyable&&) const volatile &, int const volatile&>(cl);
    }
}

void bullet_three_four_tests() {
    {
        typedef TestClass Fn;
        Fn cl(42);
        test_b34<int&>(cl);
        test_b34<int const&>(static_cast<Fn const&>(cl));
        test_b34<int volatile&>(static_cast<Fn volatile&>(cl));
        test_b34<int const volatile&>(static_cast<Fn const volatile &>(cl));

        test_b34<int&&>(static_cast<Fn &&>(cl));
        test_b34<int const&&>(static_cast<Fn const&&>(cl));
        test_b34<int volatile&&>(static_cast<Fn volatile&&>(cl));
        test_b34<int const volatile&&>(static_cast<Fn const volatile&&>(cl));
    }
    {
        typedef DerivedFromTestClass Fn;
        Fn cl(42);
        test_b34<int&>(cl);
        test_b34<int const&>(static_cast<Fn const&>(cl));
        test_b34<int volatile&>(static_cast<Fn volatile&>(cl));
        test_b34<int const volatile&>(static_cast<Fn const volatile &>(cl));

        test_b34<int&&>(static_cast<Fn &&>(cl));
        test_b34<int const&&>(static_cast<Fn const&&>(cl));
        test_b34<int volatile&&>(static_cast<Fn volatile&&>(cl));
        test_b34<int const volatile&&>(static_cast<Fn const volatile&&>(cl));
    }
    {
        typedef TestClass Fn;
        Fn cl(42);
        test_b34<int&>(std::reference_wrapper<Fn>(cl));
        test_b34<int const&>(std::reference_wrapper<Fn const>(cl));
        test_b34<int volatile&>(std::reference_wrapper<Fn volatile>(cl));
        test_b34<int const volatile&>(std::reference_wrapper<Fn const volatile>(cl));
    }
    {
        typedef DerivedFromTestClass Fn;
        Fn cl(42);
        test_b34<int&>(std::reference_wrapper<Fn>(cl));
        test_b34<int const&>(std::reference_wrapper<Fn const>(cl));
        test_b34<int volatile&>(std::reference_wrapper<Fn volatile>(cl));
        test_b34<int const volatile&>(std::reference_wrapper<Fn const volatile>(cl));
    }
    {
        typedef TestClass Fn;
        Fn cl_obj(42);
        Fn* cl = &cl_obj;
        test_b34<int&>(cl);
        test_b34<int const&>(static_cast<Fn const*>(cl));
        test_b34<int volatile&>(static_cast<Fn volatile*>(cl));
        test_b34<int const volatile&>(static_cast<Fn const volatile *>(cl));
    }
    {
        typedef DerivedFromTestClass Fn;
        Fn cl_obj(42);
        Fn* cl = &cl_obj;
        test_b34<int&>(cl);
        test_b34<int const&>(static_cast<Fn const*>(cl));
        test_b34<int volatile&>(static_cast<Fn volatile*>(cl));
        test_b34<int const volatile&>(static_cast<Fn const volatile *>(cl));
    }
}

void bullet_five_tests() {
    using FooType = int&(NonCopyable&&);
    {
        FooType& fn = foo;
        test_b5<int &>(fn);
    }
    {
        FooType* fn = foo;
        test_b5<int &>(fn);
    }
    {
        typedef TestClass Fn;
        Fn cl(42);
        test_b5<int&>(cl);
        test_b5<int const&>(static_cast<Fn const&>(cl));
        test_b5<int volatile&>(static_cast<Fn volatile&>(cl));
        test_b5<int const volatile&>(static_cast<Fn const volatile &>(cl));

        test_b5<int&&>(static_cast<Fn &&>(cl));
        test_b5<int const&&>(static_cast<Fn const&&>(cl));
        test_b5<int volatile&&>(static_cast<Fn volatile&&>(cl));
        test_b5<int const volatile&&>(static_cast<Fn const volatile&&>(cl));
    }
}

struct CopyThrows {
  CopyThrows() {}
  CopyThrows(CopyThrows const&) {}
  CopyThrows(CopyThrows&&) noexcept {}
};

struct NoThrowCallable {
  void operator()() noexcept {}
  void operator()(CopyThrows) noexcept {}
};

struct ThrowsCallable {
  void operator()() {}
};

struct MemberObj {
  int x;
};

void noexcept_test() {
    {
        NoThrowCallable obj; ((void)obj); // suppress unused warning
        CopyThrows arg; ((void)arg); // suppress unused warning
        static_assert(noexcept(std::invoke(obj)), "");
        static_assert(!noexcept(std::invoke(obj, arg)), "");
        static_assert(noexcept(std::invoke(obj, std::move(arg))), "");
    }
    {
        ThrowsCallable obj; ((void)obj); // suppress unused warning
        static_assert(!noexcept(std::invoke(obj)), "");
    }
    {
        MemberObj obj{42}; ((void)obj); // suppress unused warning.
        static_assert(noexcept(std::invoke(&MemberObj::x, obj)), "");
    }
}

int main() {
    bullet_one_two_tests();
    bullet_three_four_tests();
    bullet_five_tests();
    noexcept_test();
}
