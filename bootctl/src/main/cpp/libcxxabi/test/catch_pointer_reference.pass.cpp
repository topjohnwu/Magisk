//===---------------------- catch_pointer_referece.cpp --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This test case checks specifically the cases under bullet 3.1 & 3.2:
//
//  C++ ABI 15.3:
//  A handler is a match for an exception object of type E if
//     *  The handler is of type cv T or cv T& and E and T are the same type
//        (ignoring the top-level cv-qualifiers), or
//     *  the handler is of type cv T or cv T& and T is an unambiguous base
//        class of E, or
//  /  *  the handler is of type cv1 T* cv2 and E is a pointer type that can   \
//  |     be converted to the type of the handler by either or both of         |
//  |       o  a standard pointer conversion (4.10 [conv.ptr]) not involving   |
//  |          conversions to private or protected or ambiguous classes        |
//  \       o  a qualification conversion                                      /
//     *  the handler is a pointer or pointer to member type and E is
//        std::nullptr_t
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: libcxxabi-no-exceptions

#include <exception>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

struct Base {};
struct Derived  : Base {};
struct Derived2 : Base {};
struct Ambiguous : Derived, Derived2 {};
struct Private : private Base {};
struct Protected : protected Base {};

template <typename T  // Handler type
         ,typename E  // Thrown exception type
         ,typename O  // Object type
         >
void assert_catches()
{
    try
    {
        O o;
        throw static_cast<E>(&o);
        printf("%s\n", __PRETTY_FUNCTION__);
        assert(false && "Statements after throw must be unreachable");
    }
    catch (T t)
    {
        assert(true);
        return;
    }
    catch (...)
    {
        printf("%s\n", __PRETTY_FUNCTION__);
        assert(false && "Should not have entered catch-all");
    }

    printf("%s\n", __PRETTY_FUNCTION__);
    assert(false && "The catch should have returned");
}

template <typename T  // Handler type
         ,typename E  // Thrown exception type
         ,typename O  // Object type
         >
void assert_cannot_catch()
{
    try
    {
        O o;
        throw static_cast<E>(&o);
        printf("%s\n", __PRETTY_FUNCTION__);
        assert(false && "Statements after throw must be unreachable");
    }
    catch (T t)
    {
        printf("%s\n", __PRETTY_FUNCTION__);
        assert(false && "Should not have entered the catch");
    }
    catch (...)
    {
        assert(true);
        return;
    }

    printf("%s\n", __PRETTY_FUNCTION__);
    assert(false && "The catch-all should have returned");
}

void f1()
{
    // Test that every combination of handler of type:
    //   cv1 Base * cv2
    // catches an exception of type:
    //   Derived *
    assert_catches<               Base *               , Derived *, Derived>();
    assert_catches<const          Base *               , Derived *, Derived>();
    assert_catches<      volatile Base *               , Derived *, Derived>();
    assert_catches<const volatile Base *               , Derived *, Derived>();
    assert_catches<               Base * const         , Derived *, Derived>();
    assert_catches<const          Base * const         , Derived *, Derived>();
    assert_catches<      volatile Base * const         , Derived *, Derived>();
    assert_catches<const volatile Base * const         , Derived *, Derived>();
    assert_catches<               Base *       volatile, Derived *, Derived>();
    assert_catches<const          Base *       volatile, Derived *, Derived>();
    assert_catches<      volatile Base *       volatile, Derived *, Derived>();
    assert_catches<const volatile Base *       volatile, Derived *, Derived>();
    assert_catches<               Base * const volatile, Derived *, Derived>();
    assert_catches<const          Base * const volatile, Derived *, Derived>();
    assert_catches<      volatile Base * const volatile, Derived *, Derived>();
    assert_catches<const volatile Base * const volatile, Derived *, Derived>();
}

void f2()
{
    // Test that every combination of handler of type:
    //   cv1 Base * cv2
    // catches an exception of type:
    //   Base *
    assert_catches<               Base *               , Base *, Derived>();
    assert_catches<const          Base *               , Base *, Derived>();
    assert_catches<      volatile Base *               , Base *, Derived>();
    assert_catches<const volatile Base *               , Base *, Derived>();
    assert_catches<               Base * const         , Base *, Derived>();
    assert_catches<const          Base * const         , Base *, Derived>();
    assert_catches<      volatile Base * const         , Base *, Derived>();
    assert_catches<const volatile Base * const         , Base *, Derived>();
    assert_catches<               Base *       volatile, Base *, Derived>();
    assert_catches<const          Base *       volatile, Base *, Derived>();
    assert_catches<      volatile Base *       volatile, Base *, Derived>();
    assert_catches<const volatile Base *       volatile, Base *, Derived>();
    assert_catches<               Base * const volatile, Base *, Derived>();
    assert_catches<const          Base * const volatile, Base *, Derived>();
    assert_catches<      volatile Base * const volatile, Base *, Derived>();
    assert_catches<const volatile Base * const volatile, Base *, Derived>();
}

void f3()
{
    // Test that every combination of handler of type:
    //   cv1 Derived * cv2
    // catches an exception of type:
    //   Derived *
    assert_catches<               Derived *               , Derived *, Derived>();
    assert_catches<const          Derived *               , Derived *, Derived>();
    assert_catches<      volatile Derived *               , Derived *, Derived>();
    assert_catches<const volatile Derived *               , Derived *, Derived>();
    assert_catches<               Derived * const         , Derived *, Derived>();
    assert_catches<const          Derived * const         , Derived *, Derived>();
    assert_catches<      volatile Derived * const         , Derived *, Derived>();
    assert_catches<const volatile Derived * const         , Derived *, Derived>();
    assert_catches<               Derived *       volatile, Derived *, Derived>();
    assert_catches<const          Derived *       volatile, Derived *, Derived>();
    assert_catches<      volatile Derived *       volatile, Derived *, Derived>();
    assert_catches<const volatile Derived *       volatile, Derived *, Derived>();
    assert_catches<               Derived * const volatile, Derived *, Derived>();
    assert_catches<const          Derived * const volatile, Derived *, Derived>();
    assert_catches<      volatile Derived * const volatile, Derived *, Derived>();
    assert_catches<const volatile Derived * const volatile, Derived *, Derived>();
}

void f4()
{
    // Test that every combination of handler of type:
    //   cv1 Derived * cv2
    // cannot catch an exception of type:
    //   Base *
    assert_cannot_catch<               Derived *               , Base *, Derived>();
    assert_cannot_catch<const          Derived *               , Base *, Derived>();
    assert_cannot_catch<      volatile Derived *               , Base *, Derived>();
    assert_cannot_catch<const volatile Derived *               , Base *, Derived>();
    assert_cannot_catch<               Derived * const         , Base *, Derived>();
    assert_cannot_catch<const          Derived * const         , Base *, Derived>();
    assert_cannot_catch<      volatile Derived * const         , Base *, Derived>();
    assert_cannot_catch<const volatile Derived * const         , Base *, Derived>();
    assert_cannot_catch<               Derived *       volatile, Base *, Derived>();
    assert_cannot_catch<const          Derived *       volatile, Base *, Derived>();
    assert_cannot_catch<      volatile Derived *       volatile, Base *, Derived>();
    assert_cannot_catch<const volatile Derived *       volatile, Base *, Derived>();
    assert_cannot_catch<               Derived * const volatile, Base *, Derived>();
    assert_cannot_catch<const          Derived * const volatile, Base *, Derived>();
    assert_cannot_catch<      volatile Derived * const volatile, Base *, Derived>();
    assert_cannot_catch<const volatile Derived * const volatile, Base *, Derived>();
}

void f5()
{
    // Test that every combination of handler of type:
    //   cv1 Derived * cv2 &
    // catches an exception of type:
    //   Derived *
    assert_catches<               Derived *                &, Derived *, Derived>();
    assert_catches<const          Derived *                &, Derived *, Derived>();
    assert_catches<      volatile Derived *                &, Derived *, Derived>();
    assert_catches<const volatile Derived *                &, Derived *, Derived>();
    assert_catches<               Derived * const          &, Derived *, Derived>();
    assert_catches<const          Derived * const          &, Derived *, Derived>();
    assert_catches<      volatile Derived * const          &, Derived *, Derived>();
    assert_catches<const volatile Derived * const          &, Derived *, Derived>();
    assert_catches<               Derived *       volatile &, Derived *, Derived>();
    assert_catches<const          Derived *       volatile &, Derived *, Derived>();
    assert_catches<      volatile Derived *       volatile &, Derived *, Derived>();
    assert_catches<const volatile Derived *       volatile &, Derived *, Derived>();
    assert_catches<               Derived * const volatile &, Derived *, Derived>();
    assert_catches<const          Derived * const volatile &, Derived *, Derived>();
    assert_catches<      volatile Derived * const volatile &, Derived *, Derived>();
    assert_catches<const volatile Derived * const volatile &, Derived *, Derived>();
}

void f6()
{
    // Test that every combination of handler of type:
    //   cv1 Base * cv2 &
    // catches an exception of type:
    //   Base *
    assert_catches<               Base *                &, Base *, Derived>();
    assert_catches<const          Base *                &, Base *, Derived>();
    assert_catches<      volatile Base *                &, Base *, Derived>();
    assert_catches<const volatile Base *                &, Base *, Derived>();
    assert_catches<               Base * const          &, Base *, Derived>();
    assert_catches<const          Base * const          &, Base *, Derived>();
    assert_catches<      volatile Base * const          &, Base *, Derived>();
    assert_catches<const volatile Base * const          &, Base *, Derived>();
    assert_catches<               Base *       volatile &, Base *, Derived>();
    assert_catches<const          Base *       volatile &, Base *, Derived>();
    assert_catches<      volatile Base *       volatile &, Base *, Derived>();
    assert_catches<const volatile Base *       volatile &, Base *, Derived>();
    assert_catches<               Base * const volatile &, Base *, Derived>();
    assert_catches<const          Base * const volatile &, Base *, Derived>();
    assert_catches<      volatile Base * const volatile &, Base *, Derived>();
    assert_catches<const volatile Base * const volatile &, Base *, Derived>();

}

void f7()
{
    // Test that every combination of handler of type:
    //   cv1 Derived * cv2 &
    // cannot catch an exception of type:
    //   Base *
    assert_cannot_catch<               Derived *                &, Base *, Derived>();
    assert_cannot_catch<const          Derived *                &, Base *, Derived>();
    assert_cannot_catch<      volatile Derived *                &, Base *, Derived>();
    assert_cannot_catch<const volatile Derived *                &, Base *, Derived>();
    assert_cannot_catch<               Derived * const          &, Base *, Derived>();
    assert_cannot_catch<const          Derived * const          &, Base *, Derived>();
    assert_cannot_catch<      volatile Derived * const          &, Base *, Derived>();
    assert_cannot_catch<const volatile Derived * const          &, Base *, Derived>();
    assert_cannot_catch<               Derived *       volatile &, Base *, Derived>();
    assert_cannot_catch<const          Derived *       volatile &, Base *, Derived>();
    assert_cannot_catch<      volatile Derived *       volatile &, Base *, Derived>();
    assert_cannot_catch<const volatile Derived *       volatile &, Base *, Derived>();
    assert_cannot_catch<               Derived * const volatile &, Base *, Derived>();
    assert_cannot_catch<const          Derived * const volatile &, Base *, Derived>();
    assert_cannot_catch<      volatile Derived * const volatile &, Base *, Derived>();
    assert_cannot_catch<const volatile Derived * const volatile &, Base *, Derived>();
}

void f8()
{
    // This test case has a caveat noted in the discussion here:
    //   https://gcc.gnu.org/ml/gcc-patches/2009-08/msg00264.html
    // Specifically:
    //   This [test exposes a] corner case of the ARM C++ ABI. The generic C++
    //   ABI also gets this wrong, because I failed to notice the subtlety here.
    //   The issue is that 15.3/3 3rd bullet says:
    //     The handler is of type cv1 T* cv2 and E is a pointer type that
    //     can be converted to the type of the handler by either or both of:
    //       * a standard pointer conversion (4.10) not involving conversions
    //         to pointers to private or protected or ambiguous classes
    //   Notice that the handlers of type "cv1 T*cv2&" are not allowed such
    //   freedom to find a base class. The ABI error is that we treat handlers
    //   of reference type exactly the same as the corresponding hander of
    //   non-reference type. Elsewhere in the exception handling this makes no
    //   difference (for instance bullet 1 explicitly says 'cv T or cv T&').
    //
    // See also: http://www.open-std.org/jtc1/sc22/wg21/docs/cwg_defects.html#388
    //
    //  TL;DR: it is an unresolved C++ ABI defect that these do catch

    // Test that every combination of handler of type:
    //   cv1 Base * cv2 &
    // catches an exception of type:
    //   Derived *
    assert_catches<               Base *                &, Derived *, Derived>();
    assert_catches<const          Base *                &, Derived *, Derived>();
    assert_catches<      volatile Base *                &, Derived *, Derived>();
    assert_catches<const volatile Base *                &, Derived *, Derived>();
    assert_catches<               Base * const          &, Derived *, Derived>();
    assert_catches<const          Base * const          &, Derived *, Derived>();
    assert_catches<      volatile Base * const          &, Derived *, Derived>();
    assert_catches<const volatile Base * const          &, Derived *, Derived>();
    assert_catches<               Base *       volatile &, Derived *, Derived>();
    assert_catches<const          Base *       volatile &, Derived *, Derived>();
    assert_catches<      volatile Base *       volatile &, Derived *, Derived>();
    assert_catches<const volatile Base *       volatile &, Derived *, Derived>();
    assert_catches<               Base * const volatile &, Derived *, Derived>();
    assert_catches<const          Base * const volatile &, Derived *, Derived>();
    assert_catches<      volatile Base * const volatile &, Derived *, Derived>();
    assert_catches<const volatile Base * const volatile &, Derived *, Derived>();
}

void f9()
{
    // Test that every combination of handler of type:
    //   cv1 Base * cv2
    // cannot catch an exception of type:
    //   Ambiguous *
    assert_cannot_catch<               Base *               , Ambiguous *, Ambiguous>();
    assert_cannot_catch<const          Base *               , Ambiguous *, Ambiguous>();
    assert_cannot_catch<      volatile Base *               , Ambiguous *, Ambiguous>();
    assert_cannot_catch<const volatile Base *               , Ambiguous *, Ambiguous>();
    assert_cannot_catch<               Base * const         , Ambiguous *, Ambiguous>();
    assert_cannot_catch<const          Base * const         , Ambiguous *, Ambiguous>();
    assert_cannot_catch<      volatile Base * const         , Ambiguous *, Ambiguous>();
    assert_cannot_catch<const volatile Base * const         , Ambiguous *, Ambiguous>();
    assert_cannot_catch<               Base *       volatile, Ambiguous *, Ambiguous>();
    assert_cannot_catch<const          Base *       volatile, Ambiguous *, Ambiguous>();
    assert_cannot_catch<      volatile Base *       volatile, Ambiguous *, Ambiguous>();
    assert_cannot_catch<const volatile Base *       volatile, Ambiguous *, Ambiguous>();
    assert_cannot_catch<               Base * const volatile, Ambiguous *, Ambiguous>();
    assert_cannot_catch<const          Base * const volatile, Ambiguous *, Ambiguous>();
    assert_cannot_catch<      volatile Base * const volatile, Ambiguous *, Ambiguous>();
    assert_cannot_catch<const volatile Base * const volatile, Ambiguous *, Ambiguous>();
}

void f10()
{
    // Test that every combination of handler of type:
    //  cv1 Base * cv2
    // cannot catch an exception of type:
    //  Private *
    assert_cannot_catch<               Base *               , Private *, Private>();
    assert_cannot_catch<const          Base *               , Private *, Private>();
    assert_cannot_catch<      volatile Base *               , Private *, Private>();
    assert_cannot_catch<const volatile Base *               , Private *, Private>();
    assert_cannot_catch<               Base * const         , Private *, Private>();
    assert_cannot_catch<const          Base * const         , Private *, Private>();
    assert_cannot_catch<      volatile Base * const         , Private *, Private>();
    assert_cannot_catch<const volatile Base * const         , Private *, Private>();
    assert_cannot_catch<               Base *       volatile, Private *, Private>();
    assert_cannot_catch<const          Base *       volatile, Private *, Private>();
    assert_cannot_catch<      volatile Base *       volatile, Private *, Private>();
    assert_cannot_catch<const volatile Base *       volatile, Private *, Private>();
    assert_cannot_catch<               Base * const volatile, Private *, Private>();
    assert_cannot_catch<const          Base * const volatile, Private *, Private>();
    assert_cannot_catch<      volatile Base * const volatile, Private *, Private>();
    assert_cannot_catch<const volatile Base * const volatile, Private *, Private>();
}

void f11()
{
    // Test that every combination of handler of type:
    //  cv1 Base * cv2
    // cannot catch an exception of type:
    //  Protected *
    assert_cannot_catch<               Base *               , Protected *, Protected>();
    assert_cannot_catch<const          Base *               , Protected *, Protected>();
    assert_cannot_catch<      volatile Base *               , Protected *, Protected>();
    assert_cannot_catch<const volatile Base *               , Protected *, Protected>();
    assert_cannot_catch<               Base * const         , Protected *, Protected>();
    assert_cannot_catch<const          Base * const         , Protected *, Protected>();
    assert_cannot_catch<      volatile Base * const         , Protected *, Protected>();
    assert_cannot_catch<const volatile Base * const         , Protected *, Protected>();
    assert_cannot_catch<               Base *       volatile, Protected *, Protected>();
    assert_cannot_catch<const          Base *       volatile, Protected *, Protected>();
    assert_cannot_catch<      volatile Base *       volatile, Protected *, Protected>();
    assert_cannot_catch<const volatile Base *       volatile, Protected *, Protected>();
    assert_cannot_catch<               Base * const volatile, Protected *, Protected>();
    assert_cannot_catch<const          Base * const volatile, Protected *, Protected>();
    assert_cannot_catch<      volatile Base * const volatile, Protected *, Protected>();
    assert_cannot_catch<const volatile Base * const volatile, Protected *, Protected>();
}

void f12()
{
    // Test that every combination of handler of type:
    //  cv1 Base * cv2 &
    // cannot catch an exception of type:
    //  Private *
    assert_cannot_catch<               Base *                &, Private *, Private>();
    assert_cannot_catch<const          Base *                &, Private *, Private>();
    assert_cannot_catch<      volatile Base *                &, Private *, Private>();
    assert_cannot_catch<const volatile Base *                &, Private *, Private>();
    assert_cannot_catch<               Base * const          &, Private *, Private>();
    assert_cannot_catch<const          Base * const          &, Private *, Private>();
    assert_cannot_catch<      volatile Base * const          &, Private *, Private>();
    assert_cannot_catch<const volatile Base * const          &, Private *, Private>();
    assert_cannot_catch<               Base *       volatile &, Private *, Private>();
    assert_cannot_catch<const          Base *       volatile &, Private *, Private>();
    assert_cannot_catch<      volatile Base *       volatile &, Private *, Private>();
    assert_cannot_catch<const volatile Base *       volatile &, Private *, Private>();
    assert_cannot_catch<               Base * const volatile &, Private *, Private>();
    assert_cannot_catch<const          Base * const volatile &, Private *, Private>();
    assert_cannot_catch<      volatile Base * const volatile &, Private *, Private>();
    assert_cannot_catch<const volatile Base * const volatile &, Private *, Private>();
}

void f13()
{
    // Test that every combination of handler of type:
    //  cv1 Base * cv2 &
    // cannot catch an exception of type:
    //  Protected *
    assert_cannot_catch<               Base *                &, Protected *, Protected>();
    assert_cannot_catch<const          Base *                &, Protected *, Protected>();
    assert_cannot_catch<      volatile Base *                &, Protected *, Protected>();
    assert_cannot_catch<const volatile Base *                &, Protected *, Protected>();
    assert_cannot_catch<               Base * const          &, Protected *, Protected>();
    assert_cannot_catch<const          Base * const          &, Protected *, Protected>();
    assert_cannot_catch<      volatile Base * const          &, Protected *, Protected>();
    assert_cannot_catch<const volatile Base * const          &, Protected *, Protected>();
    assert_cannot_catch<               Base *       volatile &, Protected *, Protected>();
    assert_cannot_catch<const          Base *       volatile &, Protected *, Protected>();
    assert_cannot_catch<      volatile Base *       volatile &, Protected *, Protected>();
    assert_cannot_catch<const volatile Base *       volatile &, Protected *, Protected>();
    assert_cannot_catch<               Base * const volatile &, Protected *, Protected>();
    assert_cannot_catch<const          Base * const volatile &, Protected *, Protected>();
    assert_cannot_catch<      volatile Base * const volatile &, Protected *, Protected>();
    assert_cannot_catch<const volatile Base * const volatile &, Protected *, Protected>();
}

int main()
{
    f1();
    f2();
    f3();
    f4();
    f5();
    f6();
    f7();
    f8();
    f9();
    f10();
    f11();
    f12();
    f13();
}
