//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <cstddef>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

// typedef decltype(nullptr) nullptr_t;

struct A
{
    A(std::nullptr_t) {}
};

template <class T>
void test_conversions()
{
    {
        T p = 0;
        assert(p == nullptr);
    }
    {
        T p = nullptr;
        assert(p == nullptr);
        assert(nullptr == p);
        assert(!(p != nullptr));
        assert(!(nullptr != p));
    }
}

template <class T> struct Voider { typedef void type; };
template <class T, class = void> struct has_less : std::false_type {};

template <class T> struct has_less<T,
    typename Voider<decltype(std::declval<T>() < nullptr)>::type> : std::true_type {};

template <class T>
void test_comparisons()
{
    T p = nullptr;
    assert(p == nullptr);
    assert(!(p != nullptr));
    assert(nullptr == p);
    assert(!(nullptr != p));
}

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnull-conversion"
#endif
void test_nullptr_conversions() {
// GCC does not accept this due to CWG Defect #1423
// http://www.open-std.org/jtc1/sc22/wg21/docs/cwg_defects.html#1423
#if defined(__clang__)
    {
        bool b = nullptr;
        assert(!b);
    }
#endif
    {
        bool b(nullptr);
        assert(!b);
    }
}
#if defined(__clang__)
#pragma clang diagnostic pop
#endif


int main()
{
    static_assert(sizeof(std::nullptr_t) == sizeof(void*),
                  "sizeof(std::nullptr_t) == sizeof(void*)");

    {
        test_conversions<std::nullptr_t>();
        test_conversions<void*>();
        test_conversions<A*>();
        test_conversions<void(*)()>();
        test_conversions<void(A::*)()>();
        test_conversions<int A::*>();
    }
    {
#ifdef _LIBCPP_HAS_NO_NULLPTR
        static_assert(!has_less<std::nullptr_t>::value, "");
        // FIXME: our C++03 nullptr emulation still allows for comparisons
        // with other pointer types by way of the conversion operator.
        //static_assert(!has_less<void*>::value, "");
#else
        // TODO Enable this assertion when all compilers implement core DR 583.
        // static_assert(!has_less<std::nullptr_t>::value, "");
#endif
        test_comparisons<std::nullptr_t>();
        test_comparisons<void*>();
        test_comparisons<A*>();
        test_comparisons<void(*)()>();
    }
    test_nullptr_conversions();
}
