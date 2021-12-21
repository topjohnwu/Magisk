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

// void rethrow_nested [[noreturn]] () const;

#include <exception>
#include <cstdlib>
#include <cassert>

class A
{
    int data_;
public:
    explicit A(int data) : data_(data) {}

    friend bool operator==(const A& x, const A& y) {return x.data_ == y.data_;}
};

void go_quietly()
{
    std::exit(0);
}

int main()
{
    {
        try
        {
            throw A(2);
            assert(false);
        }
        catch (const A&)
        {
            const std::nested_exception e;
            assert(e.nested_ptr() != nullptr);
            try
            {
                e.rethrow_nested();
                assert(false);
            }
            catch (const A& a)
            {
                assert(a == A(2));
            }
        }
    }
    {
        try
        {
            std::set_terminate(go_quietly);
            const std::nested_exception e;
            e.rethrow_nested();
            assert(false);
        }
        catch (...)
        {
            assert(false);
        }
    }
}
