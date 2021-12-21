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

// shared_lock& operator=(shared_lock&& u);

#include <shared_mutex>
#include <cassert>
#include "nasty_containers.hpp"


int main()
{
    {
    typedef std::shared_timed_mutex M;
    M m0;
    M m1;
    std::shared_lock<M> lk0(m0);
    std::shared_lock<M> lk1(m1);
    lk1 = std::move(lk0);
    assert(lk1.mutex() == std::addressof(m0));
    assert(lk1.owns_lock() == true);
    assert(lk0.mutex() == nullptr);
    assert(lk0.owns_lock() == false);
    }
    {
    typedef nasty_mutex M;
    M m0;
    M m1;
    std::shared_lock<M> lk0(m0);
    std::shared_lock<M> lk1(m1);
    lk1 = std::move(lk0);
    assert(lk1.mutex() == std::addressof(m0));
    assert(lk1.owns_lock() == true);
    assert(lk0.mutex() == nullptr);
    assert(lk0.owns_lock() == false);
    }
}
