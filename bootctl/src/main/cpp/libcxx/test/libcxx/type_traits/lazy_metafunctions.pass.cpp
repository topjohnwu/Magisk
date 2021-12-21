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

// <type_traits>

// __lazy_enable_if, __lazy_not, __lazy_and and __lazy_or

// Test the libc++ lazy meta-programming helpers in <type_traits>

#include <type_traits>

template <class Type>
struct Identity {
    typedef Type type;
};

typedef std::true_type TrueT;
typedef std::false_type FalseT;

typedef Identity<TrueT>  LazyTrueT;
typedef Identity<FalseT> LazyFalseT;

// A type that cannot be instantiated
template <class T>
struct CannotInst {
    typedef T type;
    static_assert(std::is_same<T, T>::value == false, "");
};


template <int Value>
struct NextInt {
    typedef NextInt<Value + 1> type;
    static const int value = Value;
};

template <int Value>
const int NextInt<Value>::value;


template <class Type>
struct HasTypeImp {
    template <class Up, class = typename Up::type>
    static TrueT test(int);
    template <class>
    static FalseT test(...);

    typedef decltype(test<Type>(0)) type;
};

// A metafunction that returns True if Type has a nested 'type' typedef
// and false otherwise.
template <class Type>
struct HasType : HasTypeImp<Type>::type {};

void LazyEnableIfTest() {
    {
        typedef std::__lazy_enable_if<true, NextInt<0> > Result;
        static_assert(HasType<Result>::value, "");
        static_assert(Result::type::value == 1, "");
    }
    {
        typedef std::__lazy_enable_if<false, CannotInst<int> > Result;
        static_assert(!HasType<Result>::value, "");
    }
}

void LazyNotTest() {
    {
        typedef std::__lazy_not<LazyTrueT> NotT;
        static_assert(std::is_same<typename NotT::type, FalseT>::value, "");
        static_assert(NotT::value == false, "");
    }
    {
        typedef std::__lazy_not<LazyFalseT> NotT;
        static_assert(std::is_same<typename NotT::type, TrueT>::value, "");
        static_assert(NotT::value == true, "");
    }
    {
         // Check that CannotInst<int> is not instantiated.
        typedef std::__lazy_not<CannotInst<int> > NotT;

        static_assert(std::is_same<NotT, NotT>::value, "");

    }
}

void LazyAndTest() {
    { // Test that it acts as the identity function for a single value
        static_assert(std::__lazy_and<LazyFalseT>::value == false, "");
        static_assert(std::__lazy_and<LazyTrueT>::value == true, "");
    }
    {
        static_assert(std::__lazy_and<LazyTrueT, LazyTrueT>::value == true, "");
        static_assert(std::__lazy_and<LazyTrueT, LazyFalseT>::value == false, "");
        static_assert(std::__lazy_and<LazyFalseT, LazyTrueT>::value == false, "");
        static_assert(std::__lazy_and<LazyFalseT, LazyFalseT>::value == false, "");
    }
    { // Test short circuiting - CannotInst<T> should never be instantiated.
        static_assert(std::__lazy_and<LazyFalseT, CannotInst<int>>::value == false, "");
        static_assert(std::__lazy_and<LazyTrueT, LazyFalseT, CannotInst<int>>::value == false, "");
    }
}


void LazyOrTest() {
    { // Test that it acts as the identity function for a single value
        static_assert(std::__lazy_or<LazyFalseT>::value == false, "");
        static_assert(std::__lazy_or<LazyTrueT>::value == true, "");
    }
    {
        static_assert(std::__lazy_or<LazyTrueT, LazyTrueT>::value == true, "");
        static_assert(std::__lazy_or<LazyTrueT, LazyFalseT>::value == true, "");
        static_assert(std::__lazy_or<LazyFalseT, LazyTrueT>::value == true, "");
        static_assert(std::__lazy_or<LazyFalseT, LazyFalseT>::value == false, "");
    }
    { // Test short circuiting - CannotInst<T> should never be instantiated.
        static_assert(std::__lazy_or<LazyTrueT, CannotInst<int>>::value == true, "");
        static_assert(std::__lazy_or<LazyFalseT, LazyTrueT, CannotInst<int>>::value == true, "");
    }
}


int main() {
    LazyEnableIfTest();
    LazyNotTest();
    LazyAndTest();
    LazyOrTest();
}
