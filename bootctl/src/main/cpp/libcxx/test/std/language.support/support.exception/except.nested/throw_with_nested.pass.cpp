//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: libcpp-no-exceptions
// <exception>

// class nested_exception;

// template<class T> void throw_with_nested [[noreturn]] (T&& t);

#include <exception>
#include <cstdlib>
#include <cassert>

#include "test_macros.h"

class A
{
    int data_;
public:
    explicit A(int data) : data_(data) {}

    friend bool operator==(const A& x, const A& y) {return x.data_ == y.data_;}
};

class B
    : public std::nested_exception
{
    int data_;
public:
    explicit B(int data) : data_(data) {}

    friend bool operator==(const B& x, const B& y) {return x.data_ == y.data_;}
};

#if TEST_STD_VER > 11
struct Final final {};
#endif

int main()
{
    {
        try
        {
            A a(3);
            std::throw_with_nested(a);
            assert(false);
        }
        catch (const A& a)
        {
            assert(a == A(3));
        }
    }
    {
        try
        {
            A a(4);
            std::throw_with_nested(a);
            assert(false);
        }
        catch (const std::nested_exception& e)
        {
            assert(e.nested_ptr() == nullptr);
        }
    }
    {
        try
        {
            B b(5);
            std::throw_with_nested(b);
            assert(false);
        }
        catch (const B& b)
        {
            assert(b == B(5));
        }
    }
    {
        try
        {
            B b(6);
            std::throw_with_nested(b);
            assert(false);
        }
        catch (const std::nested_exception& e)
        {
            assert(e.nested_ptr() == nullptr);
            const B& b = dynamic_cast<const B&>(e);
            assert(b == B(6));
        }
    }
    {
        try
        {
            int i = 7;
            std::throw_with_nested(i);
            assert(false);
        }
        catch (int i)
        {
            assert(i == 7);
        }
    }
    {
        try
        {
            std::throw_with_nested("String literal");
            assert(false);
        }
        catch (const char *)
        {
        }
    }
#if TEST_STD_VER > 11
    {
        try
        {
            std::throw_with_nested(Final());
            assert(false);
        }
        catch (const Final &)
        {
        }
    }
#endif
}
