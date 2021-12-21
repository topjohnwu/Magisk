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

// class promise<R>

// promise();

#include <future>
#include <cassert>

int main()
{
    {
        std::promise<int> p;
        std::future<int> f = p.get_future();
        assert(f.valid());
    }
    {
        std::promise<int&> p;
        std::future<int&> f = p.get_future();
        assert(f.valid());
    }
    {
        std::promise<void> p;
        std::future<void> f = p.get_future();
        assert(f.valid());
    }
}
