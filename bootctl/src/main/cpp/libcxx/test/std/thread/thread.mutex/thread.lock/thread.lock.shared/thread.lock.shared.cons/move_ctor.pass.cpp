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
// UNSUPPORTED: c++98, c++03, c++11

// <shared_mutex>

// template <class Mutex> class shared_lock;

// shared_lock(shared_lock&& u);

#include <shared_mutex>
#include <cassert>
#include "nasty_containers.hpp"

int main()
{
    {
    typedef std::shared_timed_mutex M;
    M m;
    std::shared_lock<M> lk0(m);
    std::shared_lock<M> lk = std::move(lk0);
    assert(lk.mutex() == std::addressof(m));
    assert(lk.owns_lock() == true);
    assert(lk0.mutex() == nullptr);
    assert(lk0.owns_lock() == false);
    }
    {
    typedef nasty_mutex M;
    M m;
    std::shared_lock<M> lk0(m);
    std::shared_lock<M> lk = std::move(lk0);
    assert(lk.mutex() == std::addressof(m));
    assert(lk.owns_lock() == true);
    assert(lk0.mutex() == nullptr);
    assert(lk0.owns_lock() == false);
    }
}
