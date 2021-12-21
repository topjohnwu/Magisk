//===--------------------- inherited_exception.cpp ------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This test case checks specifically the cases under C++ ABI 15.3.1, and 15.3.2
//
//  C++ ABI 15.3:
//  A handler is a match for an exception object of type E if
//  /  *  The handler is of type cv T or cv T& and E and T are the same type   \
//  |     (ignoring the top-level cv-qualifiers), or                           |
//  |  *  the handler is of type cv T or cv T& and T is an unambiguous base    |
//  \     class of E, or                                                       /
//     *  the handler is of type cv1 T* cv2 and E is a pointer type that can
//        be converted to the type of the handler by either or both of
//          o  a standard pointer conversion (4.10 [conv.ptr]) not involving
//             conversions to private or protected or ambiguous classes
//          o  a qualification conversion
//     *  the handler is a pointer or pointer to member type and E is
//        std::nullptr_t
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: libcxxabi-no-exceptions

// Clang emits  warnings about exceptions of type 'Child' being caught by
// an earlier handler of type 'Base'. Congrats clang, you've just
// diagnosed the behavior under test.
#if defined(__clang__)
#pragma clang diagnostic ignored "-Wexceptions"
#endif

#include <assert.h>

struct Base {
  int b1;
};

struct Base2 {
  int b2;
};

struct Child : public Base, public Base2 {
  int c;
};

void f1() {
  Child child;
  child.b1 = 10;
  child.b2 = 11;
  child.c = 12;
  throw child;
}

void f2() {
  Child child;
  child.b1 = 10;
  child.b2 = 11;
  child.c = 12;
  throw static_cast<Base2&>(child);
}

void f3() {
  static Child child;
  child.b1 = 10;
  child.b2 = 11;
  child.c = 12;
  throw static_cast<Base2*>(&child);
}

int main()
{
    try
    {
        f1();
        assert(false);
    }
    catch (const Child& c)
    {
        assert(true);
    }
    catch (const Base& b)
    {
        assert(false);
    }
    catch (...)
    {
        assert(false);
    }

    try
    {
        f1();
        assert(false);
    }
    catch (const Base& c)
    {
        assert(true);
    }
    catch (const Child& b)
    {
        assert(false);
    }
    catch (...)
    {
        assert(false);
    }

    try
    {
        f1();
        assert(false);
    }
    catch (const Base2& c)
    {
        assert(true);
    }
    catch (const Child& b)
    {
        assert(false);
    }
    catch (...)
    {
        assert(false);
    }

    try
    {
        f2();
        assert(false);
    }
    catch (const Child& c)
    {
        assert(false);
    }
    catch (const Base& b)
    {
        assert(false);
    }
    catch (const Base2& b)
    {
        assert(true);
    }
    catch (...)
    {
        assert(false);
    }

    try
    {
        f3();
        assert(false);
    }
    catch (const Base* c)
    {
        assert(false);
    }
    catch (const Child* b)
    {
        assert(false);
    }
    catch (const Base2* c)
    {
        assert(true);
    }
    catch (...)
    {
        assert(false);
    }
}
