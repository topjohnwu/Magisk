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

// <future>

// class shared_future<R>

// shared_future();

#include <future>
#include <cassert>

int main()
{
    {
        std::shared_future<int> f;
        assert(!f.valid());
    }
    {
        std::shared_future<int&> f;
        assert(!f.valid());
    }
    {
        std::shared_future<void> f;
        assert(!f.valid());
    }
}
