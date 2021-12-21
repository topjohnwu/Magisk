//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// UNSUPPORTED: libcpp-has-no-threads
// UNSUPPORTED: c++98, c++03

// <future>

// class packaged_task<R(ArgTypes...)>

// void reset();

#include <future>
#include <cassert>

#include "test_macros.h"

class A
{
    long data_;

public:
    explicit A(long i) : data_(i) {}

    long operator()(long i, long j) const
    {
        return data_ + i + j;
    }
};

int main()
{
    {
        std::packaged_task<double(int, char)> p(A(5));
        std::future<double> f = p.get_future();
        p(3, 'a');
        assert(f.get() == 105.0);
        p.reset();
        p(4, 'a');
        f = p.get_future();
        assert(f.get() == 106.0);
    }
#ifndef TEST_HAS_NO_EXCEPTIONS
    {
        std::packaged_task<double(int, char)> p;
        try
        {
            p.reset();
            assert(false);
        }
        catch (const std::future_error& e)
        {
            assert(e.code() == make_error_code(std::future_errc::no_state));
        }
    }
#endif
}
