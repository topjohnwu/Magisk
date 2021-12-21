//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>

// class function<R(ArgTypes...)>

// function(Fp);

// Ensure that __not_null works for all function types.
// See https://bugs.llvm.org/show_bug.cgi?id=23589

//------------------------------------------------------------------------------
// TESTING std::function<...>::__not_null(Callable)
//
// Concerns:
//  1) The call __not_null(Callable) is well formed and correct for each
//     possible 'Callable' type category. These categories include:
//      1a) function pointers
//      1b) member function pointer
//      1c) member data pointer
//      1d) callable class type
//      1e) lambdas
//    Categories 1a, 1b, and 1c are 'Nullable' types. Only objects of these
//    types can be null. The other categories are not tested here.
//  3) '__not_null(Callable)' is well formed when the call signature includes
//      varargs.
//  4) '__not_null(Callable)' works for Callable types with all arities less
//     than or equal to 3 in C++03.
//  5) '__not_null(Callable)' works when 'Callable' is a member function
//     pointer to a cv or ref qualified function type.
//
// Plan:
//  1 For categories 1a, 1b and 1c define a set of
//    'Callable' objects for this category. This set should include examples
//    of arity 0, 1, 2 and possible 3 including versions with varargs as the
//    last parameter.
//
//  2 For each 'Callable' object in categories 1a, 1b and 1c do the following.
//
//    1 Define a type 'std::function<Sig>' as 'F' where 'Sig' is compatible with
//      the signature of the 'Callable' object.
//
//    2 Create an object of type 'F' using a null pointer of type 'Callable'.
//      Check that 'F.target<Callable>()' is null.
//
//    3 Create an object of type 'F' that is not null. Check that
//      'F.target<Callable>()' is not null and is equal to the original
//      argument.

#include <functional>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

///////////////////////////////////////////////////////////////////////////////
int foo() { return 42; }
int foo(int) { return 42; }
int foo(int, int) { return 42; }
int foo(int, int, int) { return 42; }

int foo(...) { return 42; }
int foo(int, ...) { return 42; }
int foo(int, int, ...) { return 42; }
int foo(int, int, int, ...) { return 42; }

///////////////////////////////////////////////////////////////////////////////
struct MemFun03 {
    int foo() { return 42; }
    int foo() const { return 42; }
    int foo() volatile { return 42; }
    int foo() const volatile { return 42; }

    int foo(int) { return 42; }
    int foo(int) const { return 42; }
    int foo(int) volatile { return 42; }
    int foo(int) const volatile { return 42; }

    int foo(int, int) { return 42; }
    int foo(int, int) const { return 42; }
    int foo(int, int) volatile { return 42; }
    int foo(int, int) const volatile { return 42; }

    int foo(int, int, int) { return 42; }
    int foo(int, int, int) const { return 42; }
    int foo(int, int, int) volatile { return 42; }
    int foo(int, int, int) const volatile { return 42; }

    int foo(...) { return 42; }
    int foo(...) const { return 42; }
    int foo(...) volatile { return 42; }
    int foo(...) const volatile { return 42; }

    int foo(int, ...) { return 42; }
    int foo(int, ...) const { return 42; }
    int foo(int, ...) volatile { return 42; }
    int foo(int, ...) const volatile { return 42; }

    int foo(int, int, ...) { return 42; }
    int foo(int, int, ...) const { return 42; }
    int foo(int, int, ...) volatile { return 42; }
    int foo(int, int, ...) const volatile { return 42; }

    int foo(int, int, int, ...) { return 42; }
    int foo(int, int, int, ...) const { return 42; }
    int foo(int, int, int, ...) volatile { return 42; }
    int foo(int, int, int, ...) const volatile { return 42; }
};

#if TEST_STD_VER >= 11
struct MemFun11 {
    int foo() & { return 42; }
    int foo() const & { return 42; }
    int foo() volatile & { return 42; }
    int foo() const volatile & { return 42; }

    int foo(...) & { return 42; }
    int foo(...) const & { return 42; }
    int foo(...) volatile & { return 42; }
    int foo(...) const volatile & { return 42; }

    int foo() && { return 42; }
    int foo() const && { return 42; }
    int foo() volatile && { return 42; }
    int foo() const volatile && { return 42; }

    int foo(...) && { return 42; }
    int foo(...) const && { return 42; }
    int foo(...) volatile && { return 42; }
    int foo(...) const volatile && { return 42; }
};
#endif // TEST_STD_VER >= 11

struct MemData {
    int foo;
};

// Create a non-null free function by taking the address of
// &static_cast<Tp&>(foo);
template <class Tp>
struct Creator {
    static Tp create() {
        return &foo;
    }
};

// Create a non-null member pointer.
template <class Ret, class Class>
struct Creator<Ret Class::*> {
    typedef Ret Class::*ReturnType;
    static ReturnType create() {
        return &Class::foo;
    }
};

template <class TestFn, class Fn>
void test_imp() {
    { // Check that the null value is detected
        TestFn tf = nullptr;
        std::function<Fn> f = tf;
        assert(f.template target<TestFn>() == nullptr);
    }
    { // Check that the non-null value is detected.
        TestFn tf = Creator<TestFn>::create();
        assert(tf != nullptr);
        std::function<Fn> f = tf;
        assert(f.template target<TestFn>() != nullptr);
        assert(*f.template target<TestFn>() == tf);
    }
}

void test_func() {
    test_imp<int(*)(), int()>();
    test_imp<int(*)(...), int()>();
    test_imp<int(*)(int), int(int)>();
    test_imp<int(*)(int, ...), int(int)>();
    test_imp<int(*)(int, int), int(int, int)>();
    test_imp<int(*)(int, int, ...), int(int, int)>();
    test_imp<int(*)(int, int, int), int(int, int, int)>();
    test_imp<int(*)(int, int, int, ...), int(int, int, int)>();
}

void test_mf() {
    test_imp<int(MemFun03::*)(), int(MemFun03&)>();
    test_imp<int(MemFun03::*)(...), int(MemFun03&)>();
    test_imp<int(MemFun03::*)() const, int(MemFun03&)>();
    test_imp<int(MemFun03::*)(...) const, int(MemFun03&)>();
    test_imp<int(MemFun03::*)() volatile, int(MemFun03&)>();
    test_imp<int(MemFun03::*)(...) volatile, int(MemFun03&)>();
    test_imp<int(MemFun03::*)() const volatile, int(MemFun03&)>();
    test_imp<int(MemFun03::*)(...) const volatile, int(MemFun03&)>();

    test_imp<int(MemFun03::*)(int), int(MemFun03&, int)>();
    test_imp<int(MemFun03::*)(int, ...), int(MemFun03&, int)>();
    test_imp<int(MemFun03::*)(int) const, int(MemFun03&, int)>();
    test_imp<int(MemFun03::*)(int, ...) const, int(MemFun03&, int)>();
    test_imp<int(MemFun03::*)(int) volatile, int(MemFun03&, int)>();
    test_imp<int(MemFun03::*)(int, ...) volatile, int(MemFun03&, int)>();
    test_imp<int(MemFun03::*)(int) const volatile, int(MemFun03&, int)>();
    test_imp<int(MemFun03::*)(int, ...) const volatile, int(MemFun03&, int)>();

    test_imp<int(MemFun03::*)(int, int), int(MemFun03&, int, int)>();
    test_imp<int(MemFun03::*)(int, int, ...), int(MemFun03&, int, int)>();
    test_imp<int(MemFun03::*)(int, int) const, int(MemFun03&, int, int)>();
    test_imp<int(MemFun03::*)(int, int, ...) const, int(MemFun03&, int, int)>();
    test_imp<int(MemFun03::*)(int, int) volatile, int(MemFun03&, int, int)>();
    test_imp<int(MemFun03::*)(int, int, ...) volatile, int(MemFun03&, int, int)>();
    test_imp<int(MemFun03::*)(int, int) const volatile, int(MemFun03&, int, int)>();
    test_imp<int(MemFun03::*)(int, int, ...) const volatile, int(MemFun03&, int, int)>();

#if TEST_STD_VER >= 11
    test_imp<int(MemFun11::*)() &, int(MemFun11&)>();
    test_imp<int(MemFun11::*)(...) &, int(MemFun11&)>();
    test_imp<int(MemFun11::*)() const &, int(MemFun11&)>();
    test_imp<int(MemFun11::*)(...) const &, int(MemFun11&)>();
    test_imp<int(MemFun11::*)() volatile &, int(MemFun11&)>();
    test_imp<int(MemFun11::*)(...) volatile &, int(MemFun11&)>();
    test_imp<int(MemFun11::*)() const volatile &, int(MemFun11&)>();
    test_imp<int(MemFun11::*)(...) const volatile &, int(MemFun11&)>();

    test_imp<int(MemFun11::*)() &&, int(MemFun11&&)>();
    test_imp<int(MemFun11::*)(...) &&, int(MemFun11&&)>();
    test_imp<int(MemFun11::*)() const &&, int(MemFun11&&)>();
    test_imp<int(MemFun11::*)(...) const &&, int(MemFun11&&)>();
    test_imp<int(MemFun11::*)() volatile &&, int(MemFun11&&)>();
    test_imp<int(MemFun11::*)(...) volatile &&, int(MemFun11&&)>();
    test_imp<int(MemFun11::*)() const volatile &&, int(MemFun11&&)>();
    test_imp<int(MemFun11::*)(...) const volatile &&, int(MemFun11&&)>();
#endif
}

void test_md() {
    test_imp<int MemData::*, int(MemData&)>();
}

int main() {
    test_func();
    test_mf();
    test_md();
}
