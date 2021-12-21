//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// is_trivially_destructible

// Prevent warning when testing the Abstract test type.
#if defined(__clang__)
#pragma clang diagnostic ignored "-Wdelete-non-virtual-dtor"
#endif

#include <type_traits>
#include "test_macros.h"

template <class T>
void test_is_trivially_destructible()
{
    static_assert( std::is_trivially_destructible<T>::value, "");
    static_assert( std::is_trivially_destructible<const T>::value, "");
    static_assert( std::is_trivially_destructible<volatile T>::value, "");
    static_assert( std::is_trivially_destructible<const volatile T>::value, "");
#if TEST_STD_VER > 14
    static_assert( std::is_trivially_destructible_v<T>, "");
    static_assert( std::is_trivially_destructible_v<const T>, "");
    static_assert( std::is_trivially_destructible_v<volatile T>, "");
    static_assert( std::is_trivially_destructible_v<const volatile T>, "");
#endif
}

template <class T>
void test_is_not_trivially_destructible()
{
    static_assert(!std::is_trivially_destructible<T>::value, "");
    static_assert(!std::is_trivially_destructible<const T>::value, "");
    static_assert(!std::is_trivially_destructible<volatile T>::value, "");
    static_assert(!std::is_trivially_destructible<const volatile T>::value, "");
#if TEST_STD_VER > 14
    static_assert(!std::is_trivially_destructible_v<T>, "");
    static_assert(!std::is_trivially_destructible_v<const T>, "");
    static_assert(!std::is_trivially_destructible_v<volatile T>, "");
    static_assert(!std::is_trivially_destructible_v<const volatile T>, "");
#endif
}

struct PublicDestructor           { public:     ~PublicDestructor() {}};
struct ProtectedDestructor        { protected:  ~ProtectedDestructor() {}};
struct PrivateDestructor          { private:    ~PrivateDestructor() {}};

struct VirtualPublicDestructor           { public:    virtual ~VirtualPublicDestructor() {}};
struct VirtualProtectedDestructor        { protected: virtual ~VirtualProtectedDestructor() {}};
struct VirtualPrivateDestructor          { private:   virtual ~VirtualPrivateDestructor() {}};

struct PurePublicDestructor              { public:    virtual ~PurePublicDestructor() = 0; };
struct PureProtectedDestructor           { protected: virtual ~PureProtectedDestructor() = 0; };
struct PurePrivateDestructor             { private:   virtual ~PurePrivateDestructor() = 0; };


class Empty
{
};

union Union {};

struct bit_zero
{
    int :  0;
};

class Abstract
{
    virtual void foo() = 0;
};

class AbstractDestructor
{
    virtual ~AbstractDestructor() = 0;
};

struct A
{
    ~A();
};

int main()
{
    test_is_not_trivially_destructible<void>();
    test_is_not_trivially_destructible<A>();
    test_is_not_trivially_destructible<char[]>();
    test_is_not_trivially_destructible<VirtualPublicDestructor>();
    test_is_not_trivially_destructible<PurePublicDestructor>();

    test_is_trivially_destructible<Abstract>();
    test_is_trivially_destructible<Union>();
    test_is_trivially_destructible<Empty>();
    test_is_trivially_destructible<int&>();
    test_is_trivially_destructible<int>();
    test_is_trivially_destructible<double>();
    test_is_trivially_destructible<int*>();
    test_is_trivially_destructible<const int*>();
    test_is_trivially_destructible<char[3]>();
    test_is_trivially_destructible<bit_zero>();

#if TEST_STD_VER >= 11
    // requires access control sfinae
    test_is_not_trivially_destructible<ProtectedDestructor>();
    test_is_not_trivially_destructible<PrivateDestructor>();
    test_is_not_trivially_destructible<VirtualProtectedDestructor>();
    test_is_not_trivially_destructible<VirtualPrivateDestructor>();
    test_is_not_trivially_destructible<PureProtectedDestructor>();
    test_is_not_trivially_destructible<PurePrivateDestructor>();
#endif
}
