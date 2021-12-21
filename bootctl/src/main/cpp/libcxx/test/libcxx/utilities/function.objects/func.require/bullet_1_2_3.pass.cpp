//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>

// INVOKE (f, t1, t2, ..., tN)

//------------------------------------------------------------------------------
// TESTING INVOKE(f, t1, t2, ..., tN)
//   - Bullet 1 -- (t1.*f)(t2, ..., tN)
//   - Bullet 2 -- (t1.get().*f)(t2, ..., tN) // t1 is a reference_wrapper
//   - Bullet 3 -- ((*t1).*f)(t2, ..., tN)
//
// Overview:
//    Bullets 1, 2 and 3 handle the case where 'f' is a pointer to member function.
//    Bullet 1 only handles the cases where t1 is an object of type T or a
//    type derived from 'T'. Bullet 2 handles the case where 't1' is a reference
//    wrapper and bullet 3 handles all other cases.
//
// Concerns:
//   1) cv-qualified member function signatures are accepted.
//   2) reference qualified member function signatures are accepted.
//   3) member functions with varargs at the end are accepted.
//   4) The arguments are perfect forwarded to the member function call.
//   5) Classes that are publicly derived from 'T' are accepted as the call object
//   6) All types that dereference to T or a type derived from T can be used
//      as the call object.
//   7) Pointers to T or a type derived from T can be used as the call object.
//   8) Reference return types are properly deduced.
//   9) reference_wrappers are properly handled and unwrapped.
//
//
// Plan:
//   1) Create a class that contains a set, 'S', of non-static functions.
//     'S' should include functions that cover every single combination
//      of qualifiers and varargs for arities of 0, 1 and 2 (C-1,2,3).
//      The argument types used in the functions should be non-copyable (C-4).
//      The functions should return 'MethodID::setUncheckedCall()'.
//
//   2) Create a set of supported call object, 'Objs', of different types
//      and behaviors. (C-5,6,7)
//
//   3) Attempt to call each function, 'f', in 'S' with each call object, 'c',
//      in 'Objs'. After every attempted call to 'f' check that 'f' was
//      actually called using 'MethodID::checkCalled(<return-value>)'
//
//       3b) If 'f' is reference qualified call 'f' with the properly qualified
//       call object. Otherwise call 'f' with lvalue call objects.
//
//       3a) If 'f' is const, volatile, or cv qualified then call it with call
//       objects that are equally or less cv-qualified.

#include <functional>
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "invoke_helpers.h"

//==============================================================================
// MemFun03 - C++03 compatible set of test member functions.
struct MemFun03 {
    typedef void*& R;
#define F(...) \
    R f(__VA_ARGS__) { return MethodID<R(MemFun03::*)(__VA_ARGS__)>::setUncheckedCall(); } \
    R f(__VA_ARGS__) const { return MethodID<R(MemFun03::*)(__VA_ARGS__) const>::setUncheckedCall(); } \
    R f(__VA_ARGS__) volatile { return MethodID<R(MemFun03::*)(__VA_ARGS__) volatile>::setUncheckedCall(); } \
    R f(__VA_ARGS__) const volatile { return MethodID<R(MemFun03::*)(__VA_ARGS__) const volatile>::setUncheckedCall(); }
#
    F()
    F(...)
    F(ArgType&)
    F(ArgType&, ...)
    F(ArgType&, ArgType&)
    F(ArgType&, ArgType&, ...)
    F(ArgType&, ArgType&, ArgType&)
    F(ArgType&, ArgType&, ArgType&, ...)
#undef F
public:
    MemFun03() {}
private:
    MemFun03(MemFun03 const&);
    MemFun03& operator=(MemFun03 const&);
};


#if TEST_STD_VER >= 11

//==============================================================================
// MemFun11 - C++11 reference qualified test member functions.
struct MemFun11 {
    typedef void*& R;
    typedef MemFun11 C;
#define F(...) \
    R f(__VA_ARGS__) & { return MethodID<R(C::*)(__VA_ARGS__) &>::setUncheckedCall(); } \
    R f(__VA_ARGS__) const & { return MethodID<R(C::*)(__VA_ARGS__) const &>::setUncheckedCall(); } \
    R f(__VA_ARGS__) volatile & { return MethodID<R(C::*)(__VA_ARGS__) volatile &>::setUncheckedCall(); } \
    R f(__VA_ARGS__) const volatile & { return MethodID<R(C::*)(__VA_ARGS__) const volatile &>::setUncheckedCall(); } \
    R f(__VA_ARGS__) && { return MethodID<R(C::*)(__VA_ARGS__) &&>::setUncheckedCall(); } \
    R f(__VA_ARGS__) const && { return MethodID<R(C::*)(__VA_ARGS__) const &&>::setUncheckedCall(); } \
    R f(__VA_ARGS__) volatile && { return MethodID<R(C::*)(__VA_ARGS__) volatile &&>::setUncheckedCall(); } \
    R f(__VA_ARGS__) const volatile && { return MethodID<R(C::*)(__VA_ARGS__) const volatile &&>::setUncheckedCall(); }
#
    F()
    F(...)
    F(ArgType&&)
    F(ArgType&&, ...)
    F(ArgType&&, ArgType&&)
    F(ArgType&&, ArgType&&, ...)
    F(ArgType&&, ArgType&&, ArgType&&)
    F(ArgType&&, ArgType&&, ArgType&&, ...)
#undef F
public:
    MemFun11() {}
private:
    MemFun11(MemFun11 const&);
    MemFun11& operator=(MemFun11 const&);
};

#endif // TEST_STD_VER >= 11



//==============================================================================
// TestCase - A test case for a single member function.
//   ClassType - The type of the class being tested.
//   CallSig   - The function signature of the method being tested.
//   Arity     - the arity of 'CallSig'
//   CV        - the cv qualifiers of 'CallSig' represented as a type tag.
//   RValue    - The method is RValue qualified.
//   ArgRValue - Call the method with RValue arguments.
template <class ClassType, class CallSig, int Arity, class CV,
          bool RValue = false, bool ArgRValue = false>
struct TestCaseImp {
public:

    static void run() { TestCaseImp().doTest(); }

private:
    //==========================================================================
    // TEST DISPATCH
    void doTest() {
         // (Plan-2) Create test call objects.
        typedef ClassType T;
        typedef DerivedFromType<T> D;
        T obj;
        T* obj_ptr = &obj;
        D der;
        D* der_ptr = &der;
        DerefToType<T>   dref;
        DerefPropType<T> dref2;
        std::reference_wrapper<T> rref(obj);
        std::reference_wrapper<D> drref(der);

         // (Plan-3) Dispatch based on the CV tags.
        CV tag;
        Bool<!RValue> NotRValue;
        runTestDispatch(tag,  obj);
        runTestDispatch(tag,  der);
        runTestDispatch(tag, dref2);
        runTestDispatchIf(NotRValue, tag, dref);
        runTestDispatchIf(NotRValue, tag, obj_ptr);
        runTestDispatchIf(NotRValue, tag, der_ptr);
#if TEST_STD_VER >= 11
        runTestDispatchIf(NotRValue, tag, rref);
        runTestDispatchIf(NotRValue, tag, drref);
#endif
    }

    template <class QT, class Tp>
    void runTestDispatchIf(Bool<true>, QT q, Tp& v) {
        runTestDispatch(q, v);
    }

    template <class QT, class Tp>
    void runTestDispatchIf(Bool<false>, QT, Tp&) {
    }

    template <class Tp>
    void runTestDispatch(Q_None, Tp& v) {
        runTest(v);
    }

    template <class Tp>
    void runTestDispatch(Q_Const, Tp& v) {
        runTest(v);
        runTest(makeConst(v));
    }

    template <class Tp>
    void runTestDispatch(Q_Volatile, Tp& v) {
        runTest(v);
        runTest(makeVolatile(v));

    }

    template <class Tp>
    void runTestDispatch(Q_CV, Tp& v) {
        runTest(v);
        runTest(makeConst(v));
        runTest(makeVolatile(v));
        runTest(makeCV(v));
    }

    template <class T>
    void runTest(const std::reference_wrapper<T>& obj) {
        typedef Caster<Q_None, RValue> SCast;
        typedef Caster<Q_None, ArgRValue> ACast;
        typedef CallSig (ClassType::*MemPtr);
        // Delegate test to logic in invoke_helpers.h
        BasicTest<MethodID<MemPtr>, Arity, SCast, ACast> b;
        b.runTest( (MemPtr)&ClassType::f, obj);
    }

    template <class T>
    void runTest(T* obj) {
        typedef Caster<Q_None, RValue> SCast;
        typedef Caster<Q_None, ArgRValue> ACast;
        typedef CallSig (ClassType::*MemPtr);
        // Delegate test to logic in invoke_helpers.h
        BasicTest<MethodID<MemPtr>, Arity, SCast, ACast> b;
        b.runTest( (MemPtr)&ClassType::f, obj);
    }

    template <class Obj>
    void runTest(Obj& obj) {
        typedef Caster<Q_None, RValue> SCast;
        typedef Caster<Q_None, ArgRValue> ACast;
        typedef CallSig (ClassType::*MemPtr);
        // Delegate test to logic in invoke_helpers.h
        BasicTest<MethodID<MemPtr>, Arity, SCast, ACast> b;
        b.runTest( (MemPtr)&ClassType::f, obj);
    }
};

template <class Sig, int Arity, class CV>
struct TestCase : public TestCaseImp<MemFun03, Sig, Arity, CV> {};

#if TEST_STD_VER >= 11
template <class Sig, int Arity, class CV, bool RValue = false>
struct TestCase11 : public TestCaseImp<MemFun11, Sig, Arity, CV, RValue, true> {};
#endif

template <class Tp>
struct DerivedFromRefWrap : public std::reference_wrapper<Tp> {
  DerivedFromRefWrap(Tp& tp) : std::reference_wrapper<Tp>(tp) {}
};

#if TEST_STD_VER >= 11
void test_derived_from_ref_wrap() {
    int x = 42;
    std::reference_wrapper<int> r(x);
    std::reference_wrapper<std::reference_wrapper<int>> r2(r);
    DerivedFromRefWrap<int> d(x);
    auto get_fn = &std::reference_wrapper<int>::get;
    auto& ret = std::__invoke(get_fn, r);
    auto& cret = std::__invoke_constexpr(get_fn, r);
    assert(&ret == &x);
    assert(&cret == &x);
    auto& ret2 = std::__invoke(get_fn, d);
    auto& cret2 = std::__invoke_constexpr(get_fn, d);
    assert(&ret2 == &x);
    assert(&cret2 == &x);
    auto& ret3 = std::__invoke(get_fn, r2);
    assert(&ret3 == &x);
}
#endif

int main() {
    typedef void*& R;
    typedef ArgType A;
    TestCase<R(),                                   0, Q_None>::run();
    TestCase<R() const,                             0, Q_Const>::run();
    TestCase<R() volatile,                          0, Q_Volatile>::run();
    TestCase<R() const volatile,                    0, Q_CV>::run();
    TestCase<R(...),                                0, Q_None>::run();
    TestCase<R(...) const,                          0, Q_Const>::run();
    TestCase<R(...) volatile,                       0, Q_Volatile>::run();
    TestCase<R(...) const volatile,                 0, Q_CV>::run();
    TestCase<R(A&),                                 1, Q_None>::run();
    TestCase<R(A&) const,                           1, Q_Const>::run();
    TestCase<R(A&) volatile,                        1, Q_Volatile>::run();
    TestCase<R(A&) const volatile,                  1, Q_CV>::run();
    TestCase<R(A&, ...),                            1, Q_None>::run();
    TestCase<R(A&, ...) const,                      1, Q_Const>::run();
    TestCase<R(A&, ...) volatile,                   1, Q_Volatile>::run();
    TestCase<R(A&, ...) const volatile,             1, Q_CV>::run();
    TestCase<R(A&, A&),                             2, Q_None>::run();
    TestCase<R(A&, A&) const,                       2, Q_Const>::run();
    TestCase<R(A&, A&) volatile,                    2, Q_Volatile>::run();
    TestCase<R(A&, A&) const volatile,              2, Q_CV>::run();
    TestCase<R(A&, A&, ...),                        2, Q_None>::run();
    TestCase<R(A&, A&, ...) const,                  2, Q_Const>::run();
    TestCase<R(A&, A&, ...) volatile,               2, Q_Volatile>::run();
    TestCase<R(A&, A&, ...) const volatile,         2, Q_CV>::run();
    TestCase<R(A&, A&, A&),                         3, Q_None>::run();
    TestCase<R(A&, A&, A&) const,                   3, Q_Const>::run();
    TestCase<R(A&, A&, A&) volatile,                3, Q_Volatile>::run();
    TestCase<R(A&, A&, A&) const volatile,          3, Q_CV>::run();
    TestCase<R(A&, A&, A&, ...),                    3, Q_None>::run();
    TestCase<R(A&, A&, A&, ...) const,              3, Q_Const>::run();
    TestCase<R(A&, A&, A&, ...) volatile,           3, Q_Volatile>::run();
    TestCase<R(A&, A&, A&, ...) const volatile,     3, Q_CV>::run();

#if TEST_STD_VER >= 11
    TestCase11<R() &,                               0, Q_None>::run();
    TestCase11<R() const &,                         0, Q_Const>::run();
    TestCase11<R() volatile &,                      0, Q_Volatile>::run();
    TestCase11<R() const volatile &,                0, Q_CV>::run();
    TestCase11<R(...) &,                            0, Q_None>::run();
    TestCase11<R(...) const &,                      0, Q_Const>::run();
    TestCase11<R(...) volatile &,                   0, Q_Volatile>::run();
    TestCase11<R(...) const volatile &,             0, Q_CV>::run();
    TestCase11<R(A&&) &,                            1, Q_None>::run();
    TestCase11<R(A&&) const &,                      1, Q_Const>::run();
    TestCase11<R(A&&) volatile &,                   1, Q_Volatile>::run();
    TestCase11<R(A&&) const volatile &,             1, Q_CV>::run();
    TestCase11<R(A&&, ...) &,                       1, Q_None>::run();
    TestCase11<R(A&&, ...) const &,                 1, Q_Const>::run();
    TestCase11<R(A&&, ...) volatile &,              1, Q_Volatile>::run();
    TestCase11<R(A&&, ...) const volatile &,        1, Q_CV>::run();
    TestCase11<R(A&&, A&&) &,                       2, Q_None>::run();
    TestCase11<R(A&&, A&&) const &,                 2, Q_Const>::run();
    TestCase11<R(A&&, A&&) volatile &,              2, Q_Volatile>::run();
    TestCase11<R(A&&, A&&) const volatile &,        2, Q_CV>::run();
    TestCase11<R(A&&, A&&, ...) &,                  2, Q_None>::run();
    TestCase11<R(A&&, A&&, ...) const &,            2, Q_Const>::run();
    TestCase11<R(A&&, A&&, ...) volatile &,         2, Q_Volatile>::run();
    TestCase11<R(A&&, A&&, ...) const volatile &,   2, Q_CV>::run();
    TestCase11<R() &&,                              0, Q_None, /* RValue */ true>::run();
    TestCase11<R() const &&,                        0, Q_Const, /* RValue */ true>::run();
    TestCase11<R() volatile &&,                     0, Q_Volatile, /* RValue */ true>::run();
    TestCase11<R() const volatile &&,               0, Q_CV, /* RValue */ true>::run();
    TestCase11<R(...) &&,                           0, Q_None, /* RValue */ true>::run();
    TestCase11<R(...) const &&,                     0, Q_Const, /* RValue */ true>::run();
    TestCase11<R(...) volatile &&,                  0, Q_Volatile, /* RValue */ true>::run();
    TestCase11<R(...) const volatile &&,            0, Q_CV, /* RValue */ true>::run();
    TestCase11<R(A&&) &&,                           1, Q_None, /* RValue */ true>::run();
    TestCase11<R(A&&) const &&,                     1, Q_Const, /* RValue */ true>::run();
    TestCase11<R(A&&) volatile &&,                  1, Q_Volatile, /* RValue */ true>::run();
    TestCase11<R(A&&) const volatile &&,            1, Q_CV, /* RValue */ true>::run();
    TestCase11<R(A&&, ...) &&,                      1, Q_None, /* RValue */ true>::run();
    TestCase11<R(A&&, ...) const &&,                1, Q_Const, /* RValue */ true>::run();
    TestCase11<R(A&&, ...) volatile &&,             1, Q_Volatile, /* RValue */ true>::run();
    TestCase11<R(A&&, ...) const volatile &&,       1, Q_CV, /* RValue */ true>::run();
    TestCase11<R(A&&, A&&) &&,                      2, Q_None, /* RValue */ true>::run();
    TestCase11<R(A&&, A&&) const &&,                2, Q_Const, /* RValue */ true>::run();
    TestCase11<R(A&&, A&&) volatile &&,             2, Q_Volatile, /* RValue */ true>::run();
    TestCase11<R(A&&, A&&) const volatile &&,       2, Q_CV, /* RValue */ true>::run();
    TestCase11<R(A&&, A&&, ...) &&,                 2, Q_None, /* RValue */ true>::run();
    TestCase11<R(A&&, A&&, ...) const &&,           2, Q_Const, /* RValue */ true>::run();
    TestCase11<R(A&&, A&&, ...) volatile &&,        2, Q_Volatile, /* RValue */ true>::run();
    TestCase11<R(A&&, A&&, ...) const volatile &&,  2, Q_CV, /* RValue */ true>::run();
    TestCase11<R(A&&, A&&, A&&) &&,                 3, Q_None, /* RValue */ true>::run();
    TestCase11<R(A&&, A&&, A&&) const &&,           3, Q_Const, /* RValue */ true>::run();
    TestCase11<R(A&&, A&&, A&&) volatile &&,        3, Q_Volatile, /* RValue */ true>::run();
    TestCase11<R(A&&, A&&, A&&) const volatile &&,  3, Q_CV, /* RValue */ true>::run();
    TestCase11<R(A&&, A&&, A&&, ...)  &&,                 3, Q_None, /* RValue */ true>::run();
    TestCase11<R(A&&, A&&, A&&, ...)  const &&,           3, Q_Const, /* RValue */ true>::run();
    TestCase11<R(A&&, A&&, A&&, ...)  volatile &&,        3, Q_Volatile, /* RValue */ true>::run();
    TestCase11<R(A&&, A&&, A&&, ...)  const volatile &&,  3, Q_CV, /* RValue */ true>::run();

    test_derived_from_ref_wrap();
#endif
}
