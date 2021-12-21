//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// UNSUPPORTED: c++98, c++03
//
// <functional>
//
// result_of<Fn(ArgTypes...)>

#include <type_traits>
#include <memory>
#include <utility>
#include "test_macros.h"

struct wat
{
    wat& operator*() { return *this; }
    void foo();
};

struct F {};
struct FD : public F {};

#if TEST_STD_VER > 14
template <typename T, typename U>
struct test_invoke_result;

template <typename Fn, typename ...Args, typename Ret>
struct test_invoke_result<Fn(Args...), Ret>
{
    static void call()
    {
        static_assert(std::is_invocable<Fn, Args...>::value, "");
        static_assert(std::is_invocable_r<Ret, Fn, Args...>::value, "");
        static_assert((std::is_same<typename std::invoke_result<Fn, Args...>::type, Ret>::value), "");
        static_assert((std::is_same<std::invoke_result_t<Fn, Args...>, Ret>::value), "");
    }
};
#endif

template <class T, class U>
void test_result_of_imp()
{
    static_assert((std::is_same<typename std::result_of<T>::type, U>::value), "");
#if TEST_STD_VER > 11
    static_assert((std::is_same<std::result_of_t<T>, U>::value), "");
#endif
#if TEST_STD_VER > 14
    test_invoke_result<T, U>::call();
#endif
}

int main()
{
    {
    typedef char F::*PMD;
    test_result_of_imp<PMD(F                &), char                &>();
    test_result_of_imp<PMD(F const          &), char const          &>();
    test_result_of_imp<PMD(F volatile       &), char volatile       &>();
    test_result_of_imp<PMD(F const volatile &), char const volatile &>();

    test_result_of_imp<PMD(F                &&), char                &&>();
    test_result_of_imp<PMD(F const          &&), char const          &&>();
    test_result_of_imp<PMD(F volatile       &&), char volatile       &&>();
    test_result_of_imp<PMD(F const volatile &&), char const volatile &&>();

    test_result_of_imp<PMD(F                ), char &&>();
    test_result_of_imp<PMD(F const          ), char &&>();
    test_result_of_imp<PMD(F volatile       ), char &&>();
    test_result_of_imp<PMD(F const volatile ), char &&>();

    test_result_of_imp<PMD(FD                &), char                &>();
    test_result_of_imp<PMD(FD const          &), char const          &>();
    test_result_of_imp<PMD(FD volatile       &), char volatile       &>();
    test_result_of_imp<PMD(FD const volatile &), char const volatile &>();

    test_result_of_imp<PMD(FD                &&), char                &&>();
    test_result_of_imp<PMD(FD const          &&), char const          &&>();
    test_result_of_imp<PMD(FD volatile       &&), char volatile       &&>();
    test_result_of_imp<PMD(FD const volatile &&), char const volatile &&>();

    test_result_of_imp<PMD(FD                ), char &&>();
    test_result_of_imp<PMD(FD const          ), char &&>();
    test_result_of_imp<PMD(FD volatile       ), char &&>();
    test_result_of_imp<PMD(FD const volatile ), char &&>();

    test_result_of_imp<PMD(std::unique_ptr<F>),        char &>();
    test_result_of_imp<PMD(std::unique_ptr<F const>),  const char &>();
    test_result_of_imp<PMD(std::unique_ptr<FD>),       char &>();
    test_result_of_imp<PMD(std::unique_ptr<FD const>), const char &>();

    test_result_of_imp<PMD(std::reference_wrapper<F>),        char &>();
    test_result_of_imp<PMD(std::reference_wrapper<F const>),  const char &>();
    test_result_of_imp<PMD(std::reference_wrapper<FD>),       char &>();
    test_result_of_imp<PMD(std::reference_wrapper<FD const>), const char &>();
    }
    {
    test_result_of_imp<int (F::* (F       &)) ()                &, int> ();
    test_result_of_imp<int (F::* (F       &)) () const          &, int> ();
    test_result_of_imp<int (F::* (F       &)) () volatile       &, int> ();
    test_result_of_imp<int (F::* (F       &)) () const volatile &, int> ();
    test_result_of_imp<int (F::* (F const &)) () const          &, int> ();
    test_result_of_imp<int (F::* (F const &)) () const volatile &, int> ();
    test_result_of_imp<int (F::* (F volatile &)) () volatile       &, int> ();
    test_result_of_imp<int (F::* (F volatile &)) () const volatile &, int> ();
    test_result_of_imp<int (F::* (F const volatile &)) () const volatile &, int> ();

    test_result_of_imp<int (F::* (F       &&)) ()                &&, int> ();
    test_result_of_imp<int (F::* (F       &&)) () const          &&, int> ();
    test_result_of_imp<int (F::* (F       &&)) () volatile       &&, int> ();
    test_result_of_imp<int (F::* (F       &&)) () const volatile &&, int> ();
    test_result_of_imp<int (F::* (F const &&)) () const          &&, int> ();
    test_result_of_imp<int (F::* (F const &&)) () const volatile &&, int> ();
    test_result_of_imp<int (F::* (F volatile &&)) () volatile       &&, int> ();
    test_result_of_imp<int (F::* (F volatile &&)) () const volatile &&, int> ();
    test_result_of_imp<int (F::* (F const volatile &&)) () const volatile &&, int> ();

    test_result_of_imp<int (F::* (F       )) ()                &&, int> ();
    test_result_of_imp<int (F::* (F       )) () const          &&, int> ();
    test_result_of_imp<int (F::* (F       )) () volatile       &&, int> ();
    test_result_of_imp<int (F::* (F       )) () const volatile &&, int> ();
    test_result_of_imp<int (F::* (F const )) () const          &&, int> ();
    test_result_of_imp<int (F::* (F const )) () const volatile &&, int> ();
    test_result_of_imp<int (F::* (F volatile )) () volatile       &&, int> ();
    test_result_of_imp<int (F::* (F volatile )) () const volatile &&, int> ();
    test_result_of_imp<int (F::* (F const volatile )) () const volatile &&, int> ();
    }
    {
    test_result_of_imp<int (F::* (FD       &)) ()                &, int> ();
    test_result_of_imp<int (F::* (FD       &)) () const          &, int> ();
    test_result_of_imp<int (F::* (FD       &)) () volatile       &, int> ();
    test_result_of_imp<int (F::* (FD       &)) () const volatile &, int> ();
    test_result_of_imp<int (F::* (FD const &)) () const          &, int> ();
    test_result_of_imp<int (F::* (FD const &)) () const volatile &, int> ();
    test_result_of_imp<int (F::* (FD volatile &)) () volatile       &, int> ();
    test_result_of_imp<int (F::* (FD volatile &)) () const volatile &, int> ();
    test_result_of_imp<int (F::* (FD const volatile &)) () const volatile &, int> ();

    test_result_of_imp<int (F::* (FD       &&)) ()                &&, int> ();
    test_result_of_imp<int (F::* (FD       &&)) () const          &&, int> ();
    test_result_of_imp<int (F::* (FD       &&)) () volatile       &&, int> ();
    test_result_of_imp<int (F::* (FD       &&)) () const volatile &&, int> ();
    test_result_of_imp<int (F::* (FD const &&)) () const          &&, int> ();
    test_result_of_imp<int (F::* (FD const &&)) () const volatile &&, int> ();
    test_result_of_imp<int (F::* (FD volatile &&)) () volatile       &&, int> ();
    test_result_of_imp<int (F::* (FD volatile &&)) () const volatile &&, int> ();
    test_result_of_imp<int (F::* (FD const volatile &&)) () const volatile &&, int> ();

    test_result_of_imp<int (F::* (FD       )) ()                &&, int> ();
    test_result_of_imp<int (F::* (FD       )) () const          &&, int> ();
    test_result_of_imp<int (F::* (FD       )) () volatile       &&, int> ();
    test_result_of_imp<int (F::* (FD       )) () const volatile &&, int> ();
    test_result_of_imp<int (F::* (FD const )) () const          &&, int> ();
    test_result_of_imp<int (F::* (FD const )) () const volatile &&, int> ();
    test_result_of_imp<int (F::* (FD volatile )) () volatile       &&, int> ();
    test_result_of_imp<int (F::* (FD volatile )) () const volatile &&, int> ();
    test_result_of_imp<int (F::* (FD const volatile )) () const volatile &&, int> ();
    }
    {
    test_result_of_imp<int (F::* (std::reference_wrapper<F>))       (),       int>();
    test_result_of_imp<int (F::* (std::reference_wrapper<const F>)) () const, int>();
    test_result_of_imp<int (F::* (std::unique_ptr<F>       ))       (),       int>();
    test_result_of_imp<int (F::* (std::unique_ptr<const F> ))       () const, int>();
    }
    test_result_of_imp<decltype(&wat::foo)(wat), void>();
}
