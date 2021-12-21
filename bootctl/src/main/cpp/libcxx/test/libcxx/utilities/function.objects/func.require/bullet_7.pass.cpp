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
//   - Bullet 7 -- f(t2, ..., tN)
//
// Overview:
//    Bullet 7 handles the cases where the first argument is not a member
//   function.
//
// Concerns:
//   1) Different types of callable objects are supported. Including
//      1a) Free Function pointers and references.
//      1b) Classes which provide a call operator
//      1c) lambdas
//   2) The callable objects are perfect forwarded.
//   3) The arguments are perfect forwarded.
//   4) Signatures which include varargs are supported.
//   5) In C++03 3 extra arguments should be allowed.
//
// Plan:
//  1) Define a set of free functions, 'SF', and class types with call
//     operators, 'SC', that address concerns 4 and 5. The free functions should
//     return 'FunctionID::setUncheckedCall()' and the call operators should
//     return 'MethodID::setUncheckedCall()'.
//
//  2) For each function 'f' in 'SF' and 'SC' attempt to call 'f'
//     using the correct number of arguments and cv-ref qualifiers. Check that
//     'f' has been called using 'FunctionID::checkCall()' if 'f' is a free
//     function and 'MethodID::checkCall()' otherwise.



#include <functional>
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "invoke_helpers.h"


//==============================================================================
// freeFunction03 - A C++03 free function.
void*& freeFunction03() {
    return FunctionPtrID<void*&(), freeFunction03>::setUncheckedCall();
}

void*& freeFunction03(...) {
    return FunctionPtrID<void*&(...), freeFunction03>::setUncheckedCall();
}

template <class A0>
void*& freeFunction03(A0&) {
    return FunctionPtrID<void*&(A0&), freeFunction03>::setUncheckedCall();
}


template <class A0>
void*& freeFunction03(A0&, ...) {
    return FunctionPtrID<void*&(A0&, ...), freeFunction03>::setUncheckedCall();
}

template <class A0, class A1>
void*& freeFunction03(A0&, A1&) {
    return FunctionPtrID<void*&(A0&, A1&), freeFunction03>::setUncheckedCall();
}


template <class A0, class A1>
void*& freeFunction03(A0&, A1&, ...) {
    return FunctionPtrID<void*&(A0&, A1&, ...), freeFunction03>::setUncheckedCall();
}

template <class A0, class A1, class A2>
void*& freeFunction03(A0&, A1&, A2&) {
    return FunctionPtrID<void*&(A0&, A1&, A2&), freeFunction03>::setUncheckedCall();
}

template <class A0, class A1, class A2>
void*& freeFunction03(A0&, A1&, A2&, ...) {
    return FunctionPtrID<void*&(A0&, A1&, A2&, ...), freeFunction03>::setUncheckedCall();
}

//==============================================================================
// Functor03 - C++03 compatible functor object
struct Functor03 {
    typedef void*& R;
    typedef Functor03 C;
#define F(Args, ...) \
    __VA_ARGS__ R operator() Args { return MethodID<R(C::*) Args>::setUncheckedCall(); } \
    __VA_ARGS__ R operator() Args const { return MethodID<R(C::*) Args const>::setUncheckedCall(); } \
    __VA_ARGS__ R operator() Args volatile { return MethodID<R(C::*) Args volatile>::setUncheckedCall(); } \
    __VA_ARGS__ R operator() Args const volatile { return MethodID<R(C::*) Args const volatile>::setUncheckedCall(); }
#
    F(())
    F((A0&), template <class A0>)
    F((A0&, A1&), template <class A0, class A1>)
    F((A0&, A1&, A2&), template <class A0, class A1, class A2>)
#undef F
public:
    Functor03() {}
private:
    Functor03(Functor03 const&);
    Functor03& operator=(Functor03 const&);
};


#if TEST_STD_VER >= 11

//==============================================================================
// freeFunction11 - A C++11 free function.
template <class ...Args>
void*& freeFunction11(Args&&...) {
    return FunctionPtrID<void*&(Args&&...), freeFunction11>::setUncheckedCall();
}

template <class ...Args>
void*& freeFunction11(Args&&...,...) {
    return FunctionPtrID<void*&(Args&&...,...), freeFunction11>::setUncheckedCall();
}

//==============================================================================
// Functor11 - C++11 reference qualified test member functions.
struct Functor11 {
    typedef void*& R;
    typedef Functor11 C;

#define F(CV) \
    template <class ...Args> \
    R operator()(Args&&...) CV { return MethodID<R(C::*)(Args&&...) CV>::setUncheckedCall(); }
#
    F(&)
    F(const &)
    F(volatile &)
    F(const volatile &)
    F(&&)
    F(const &&)
    F(volatile &&)
    F(const volatile &&)
#undef F
public:
    Functor11() {}
private:
    Functor11(Functor11 const&);
    Functor11& operator=(Functor11 const&);
};

#endif // TEST_STD_VER >= 11


//==============================================================================
// TestCaseFunctorImp - A test case for an operator() class method.
//   ClassType - The type of the call object.
//   CallSig   - The function signature of the call operator being tested.
//   Arity     - the arity of 'CallSig'
//   ObjCaster - Transformation function applied to call object.
//   ArgCaster - Transformation function applied to the extra arguments.
template <class ClassType, class CallSig, int Arity,
          class ObjCaster, class ArgCaster = LValueCaster>
struct TestCaseFunctorImp {
public:
    static void run() {
        typedef MethodID<CallSig ClassType::*> MID;
        BasicTest<MID, Arity, ObjCaster, ArgCaster> t;
        typedef ClassType T;
        typedef DerivedFromType<T> D;
        T obj;
        D der;
        t.runTest(obj);
        t.runTest(der);
    }
};

//==============================================================================
// TestCaseFreeFunction - A test case for a free function.
//   CallSig   - The function signature of the free function being tested.
//   FnPtr     - The function being tested.
//   Arity     - the arity of 'CallSig'
//   ArgCaster - Transformation function to be applied to the extra arguments.
template <class CallSig, CallSig* FnPtr, int Arity, class ArgCaster>
struct TestCaseFreeFunction {
public:
    static void run() {
        typedef FunctionPtrID<CallSig, FnPtr> FID;
        BasicTest<FID, Arity, LValueCaster, ArgCaster> t;

        DerefToType<CallSig*> deref_to(FnPtr);
        DerefToType<CallSig&> deref_to_ref(*FnPtr);

        t.runTest(FnPtr);
        t.runTest(*FnPtr);
        t.runTest(deref_to);
        t.runTest(deref_to_ref);
    }
};

//==============================================================================
//                          runTest Helpers
//==============================================================================
#if TEST_STD_VER >= 11
template <class Sig, int Arity, class ArgCaster>
void runFunctionTestCase11() {
    TestCaseFreeFunction<Sig, freeFunction11, Arity, ArgCaster>();
}
#endif

template <class Sig, int Arity, class ArgCaster>
void runFunctionTestCase() {
    TestCaseFreeFunction<Sig, freeFunction03, Arity, ArgCaster>();
#if TEST_STD_VER >= 11
    runFunctionTestCase11<Sig, Arity, ArgCaster>();
#endif
}

template <class Sig, int Arity, class ObjCaster, class ArgCaster>
void runFunctorTestCase() {
    TestCaseFunctorImp<Functor03, Sig, Arity, ObjCaster, ArgCaster>::run();
}

template <class Sig, int Arity, class ObjCaster>
void runFunctorTestCase() {
    TestCaseFunctorImp<Functor03, Sig, Arity, ObjCaster>::run();
}

#if TEST_STD_VER >= 11
// runTestCase - Run a test case for C++11 class functor types
template <class Sig, int Arity, class ObjCaster, class ArgCaster = LValueCaster>
void runFunctorTestCase11() {
    TestCaseFunctorImp<Functor11, Sig, Arity, ObjCaster, ArgCaster>::run();
}
#endif

// runTestCase - Run a test case for both function and functor types.
template <class Sig, int Arity, class ArgCaster>
void runTestCase() {
    runFunctionTestCase<Sig, Arity, ArgCaster>();
    runFunctorTestCase <Sig, Arity, LValueCaster, ArgCaster>();
};

int main() {
    typedef void*& R;
    typedef ArgType A;
    typedef A const CA;

    runTestCase< R(),                                   0, LValueCaster      >();
    runTestCase< R(A&),                                 1, LValueCaster      >();
    runTestCase< R(A&, A&),                             2, LValueCaster      >();
    runTestCase< R(A&, A&, A&),                         3, LValueCaster      >();
    runTestCase< R(CA&),                                1, ConstCaster       >();
    runTestCase< R(CA&, CA&),                           2, ConstCaster       >();
    runTestCase< R(CA&, CA&, CA&),                      3, ConstCaster       >();

    runFunctionTestCase<R(...),                         0, LValueCaster      >();
    runFunctionTestCase<R(A&, ...),                     1, LValueCaster      >();
    runFunctionTestCase<R(A&, A&, ...),                 2, LValueCaster      >();
    runFunctionTestCase<R(A&, A&, A&, ...),             3, LValueCaster      >();

#if TEST_STD_VER >= 11
    runFunctionTestCase11<R(A&&),                       1, MoveCaster        >();
    runFunctionTestCase11<R(A&&, ...),                  1, MoveCaster        >();
#endif

    runFunctorTestCase<R(),                             0, LValueCaster      >();
    runFunctorTestCase<R() const,                       0, ConstCaster       >();
    runFunctorTestCase<R() volatile,                    0, VolatileCaster    >();
    runFunctorTestCase<R() const volatile,              0, CVCaster          >();
    runFunctorTestCase<R(A&),                           1, LValueCaster      >();
    runFunctorTestCase<R(A&) const,                     1, ConstCaster       >();
    runFunctorTestCase<R(A&) volatile,                  1, VolatileCaster    >();
    runFunctorTestCase<R(A&) const volatile,            1, CVCaster          >();
    runFunctorTestCase<R(A&, A&),                       2, LValueCaster      >();
    runFunctorTestCase<R(A&, A&) const,                 2, ConstCaster       >();
    runFunctorTestCase<R(A&, A&) volatile,              2, VolatileCaster    >();
    runFunctorTestCase<R(A&, A&) const volatile,        2, CVCaster          >();
    runFunctorTestCase<R(A&, A&, A&),                   3, LValueCaster      >();
    runFunctorTestCase<R(A&, A&, A&) const,             3, ConstCaster       >();
    runFunctorTestCase<R(A&, A&, A&) volatile,          3, VolatileCaster    >();
    runFunctorTestCase<R(A&, A&, A&) const volatile,    3, CVCaster          >();
    {
    typedef ConstCaster CC;
    runFunctorTestCase<R(CA&),                          1, LValueCaster,   CC>();
    runFunctorTestCase<R(CA&) const,                    1, ConstCaster,    CC>();
    runFunctorTestCase<R(CA&) volatile,                 1, VolatileCaster, CC>();
    runFunctorTestCase<R(CA&) const volatile,           1, CVCaster,       CC>();
    runFunctorTestCase<R(CA&, CA&),                     2, LValueCaster,   CC>();
    runFunctorTestCase<R(CA&, CA&) const,               2, ConstCaster,    CC>();
    runFunctorTestCase<R(CA&, CA&) volatile,            2, VolatileCaster, CC>();
    runFunctorTestCase<R(CA&, CA&) const volatile,      2, CVCaster,       CC>();
    runFunctorTestCase<R(CA&, CA&, CA&),                3, LValueCaster,   CC>();
    runFunctorTestCase<R(CA&, CA&, CA&) const,          3, ConstCaster,    CC>();
    runFunctorTestCase<R(CA&, CA&, CA&) volatile,       3, VolatileCaster, CC>();
    runFunctorTestCase<R(CA&, CA&, CA&) const volatile, 3, CVCaster,       CC>();
    }

#if TEST_STD_VER >= 11
    runFunctorTestCase11<R() &,                    0, LValueCaster          >();
    runFunctorTestCase11<R() const &,              0, ConstCaster           >();
    runFunctorTestCase11<R() volatile &,           0, VolatileCaster        >();
    runFunctorTestCase11<R() const volatile &,     0, CVCaster              >();
    runFunctorTestCase11<R() &&,                   0, MoveCaster            >();
    runFunctorTestCase11<R() const &&,             0, MoveConstCaster       >();
    runFunctorTestCase11<R() volatile &&,          0, MoveVolatileCaster    >();
    runFunctorTestCase11<R() const volatile &&,    0, MoveCVCaster          >();
    {
    typedef MoveCaster MC;
    runFunctorTestCase11<R(A&&) &,                 1, LValueCaster,       MC>();
    runFunctorTestCase11<R(A&&) const &,           1, ConstCaster,        MC>();
    runFunctorTestCase11<R(A&&) volatile &,        1, VolatileCaster,     MC>();
    runFunctorTestCase11<R(A&&) const volatile &,  1, CVCaster,           MC>();
    runFunctorTestCase11<R(A&&) &&,                1, MoveCaster,         MC>();
    runFunctorTestCase11<R(A&&) const &&,          1, MoveConstCaster,    MC>();
    runFunctorTestCase11<R(A&&) volatile &&,       1, MoveVolatileCaster, MC>();
    runFunctorTestCase11<R(A&&) const volatile &&, 1, MoveCVCaster,       MC>();
    }
#endif
}
