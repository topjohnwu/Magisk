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

// class shared_future<R>

// shared_future(const shared_future& rhs);
// noexcept in C++17

#include <future>
#include <cassert>

#include "test_macros.h"

int main()
{
    {
        typedef int T;
        std::promise<T> p;
        std::shared_future<T> f0 = p.get_future();
        std::shared_future<T> f = f0;
#if TEST_STD_VER > 14
        static_assert(noexcept(std::shared_future<T>{f0}), "" );
#endif
        assert(f0.valid());
        assert(f.valid());
    }
    {
        typedef int T;
        std::shared_future<T> f0;
        std::shared_future<T> f = f0;
        assert(!f0.valid());
        assert(!f.valid());
    }
    {
        typedef int& T;
        std::promise<T> p;
        std::shared_future<T> f0 = p.get_future();
        std::shared_future<T> f = f0;
        assert(f0.valid());
        assert(f.valid());
    }
    {
        typedef int& T;
        std::shared_future<T> f0;
        std::shared_future<T> f = std::move(f0);
        assert(!f0.valid());
        assert(!f.valid());
    }
    {
        typedef void T;
        std::promise<T> p;
        std::shared_future<T> f0 = p.get_future();
        std::shared_future<T> f = f0;
        assert(f0.valid());
        assert(f.valid());
    }
    {
        typedef void T;
        std::shared_future<T> f0;
        std::shared_future<T> f = f0;
        assert(!f0.valid());
        assert(!f.valid());
    }
}
