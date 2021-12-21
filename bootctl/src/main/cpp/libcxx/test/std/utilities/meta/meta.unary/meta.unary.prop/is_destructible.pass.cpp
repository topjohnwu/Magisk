//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// is_destructible

// Prevent warning when testing the Abstract test type.
#if defined(__clang__)
#pragma clang diagnostic ignored "-Wdelete-non-virtual-dtor"
#endif

#include <type_traits>
#include "test_macros.h"


template <class T>
void test_is_destructible()
{
    static_assert( std::is_destructible<T>::value, "");
    static_assert( std::is_destructible<const T>::value, "");
    static_assert( std::is_destructible<volatile T>::value, "");
    static_assert( std::is_destructible<const volatile T>::value, "");
#if TEST_STD_VER > 14
    static_assert( std::is_destructible_v<T>, "");
    static_assert( std::is_destructible_v<const T>, "");
    static_assert( std::is_destructible_v<volatile T>, "");
    static_assert( std::is_destructible_v<const volatile T>, "");
#endif
}

template <class T>
void test_is_not_destructible()
{
    static_assert(!std::is_destructible<T>::value, "");
    static_assert(!std::is_destructible<const T>::value, "");
    static_assert(!std::is_destructible<volatile T>::value, "");
    static_assert(!std::is_destructible<const volatile T>::value, "");
#if TEST_STD_VER > 14
    static_assert(!std::is_destructible_v<T>, "");
    static_assert(!std::is_destructible_v<const T>, "");
    static_assert(!std::is_destructible_v<volatile T>, "");
    static_assert(!std::is_destructible_v<const volatile T>, "");
#endif
}

class Empty {};

class NotEmpty
{
    virtual ~NotEmpty();
};

union Union {};

struct bit_zero
{
    int :  0;
};

struct A
{
    ~A();
};

typedef void (Function) ();

struct PublicAbstract                    { public:    virtual void foo() = 0; };
struct ProtectedAbstract                 { protected: virtual void foo() = 0; };
struct PrivateAbstract                   { private:   virtual void foo() = 0; };

struct PublicDestructor                  { public:    ~PublicDestructor() {}};
struct ProtectedDestructor               { protected: ~ProtectedDestructor() {}};
struct PrivateDestructor                 { private:   ~PrivateDestructor() {}};

struct VirtualPublicDestructor           { public:    virtual ~VirtualPublicDestructor() {}};
struct VirtualProtectedDestructor        { protected: virtual ~VirtualProtectedDestructor() {}};
struct VirtualPrivateDestructor          { private:   virtual ~VirtualPrivateDestructor() {}};

struct PurePublicDestructor              { public:    virtual ~PurePublicDestructor() = 0; };
struct PureProtectedDestructor           { protected: virtual ~PureProtectedDestructor() = 0; };
struct PurePrivateDestructor             { private:   virtual ~PurePrivateDestructor() = 0; };

#if TEST_STD_VER >= 11
struct DeletedPublicDestructor           { public:    ~DeletedPublicDestructor() = delete; };
struct DeletedProtectedDestructor        { protected: ~DeletedProtectedDestructor() = delete; };
struct DeletedPrivateDestructor          { private:   ~DeletedPrivateDestructor() = delete; };

struct DeletedVirtualPublicDestructor    { public:    virtual ~DeletedVirtualPublicDestructor() = delete; };
struct DeletedVirtualProtectedDestructor { protected: virtual ~DeletedVirtualProtectedDestructor() = delete; };
struct DeletedVirtualPrivateDestructor   { private:   virtual ~DeletedVirtualPrivateDestructor() = delete; };
#endif


int main()
{
    test_is_destructible<A>();
    test_is_destructible<int&>();
    test_is_destructible<Union>();
    test_is_destructible<Empty>();
    test_is_destructible<int>();
    test_is_destructible<double>();
    test_is_destructible<int*>();
    test_is_destructible<const int*>();
    test_is_destructible<char[3]>();
    test_is_destructible<bit_zero>();
    test_is_destructible<int[3]>();
    test_is_destructible<ProtectedAbstract>();
    test_is_destructible<PublicAbstract>();
    test_is_destructible<PrivateAbstract>();
    test_is_destructible<PublicDestructor>();
    test_is_destructible<VirtualPublicDestructor>();
    test_is_destructible<PurePublicDestructor>();

    test_is_not_destructible<int[]>();
    test_is_not_destructible<void>();
    test_is_not_destructible<Function>();

#if TEST_STD_VER >= 11
    // Test access controlled destructors
    test_is_not_destructible<ProtectedDestructor>();
    test_is_not_destructible<PrivateDestructor>();
    test_is_not_destructible<VirtualProtectedDestructor>();
    test_is_not_destructible<VirtualPrivateDestructor>();
    test_is_not_destructible<PureProtectedDestructor>();
    test_is_not_destructible<PurePrivateDestructor>();

    // Test deleted constructors
    test_is_not_destructible<DeletedPublicDestructor>();
    test_is_not_destructible<DeletedProtectedDestructor>();
    test_is_not_destructible<DeletedPrivateDestructor>();
    //test_is_not_destructible<DeletedVirtualPublicDestructor>(); // previously failed due to clang bug #20268
    test_is_not_destructible<DeletedVirtualProtectedDestructor>();
    test_is_not_destructible<DeletedVirtualPrivateDestructor>();

    // Test private destructors
    test_is_not_destructible<NotEmpty>();
#endif

}
