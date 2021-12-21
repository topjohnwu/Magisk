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

// shared_lock(mutex_type& m, defer_lock_t);

#include <shared_mutex>
#include <cassert>
#include "nasty_containers.hpp"

int main()
{
    {
    typedef std::shared_timed_mutex M;
    M m;
    std::unique_lock<M> lk(m, std::defer_lock);
    assert(lk.mutex() == std::addressof(m));
    assert(lk.owns_lock() == false);
    }
    {
    typedef nasty_mutex M;
    M m;
    std::unique_lock<M> lk(m, std::defer_lock);
    assert(lk.mutex() == std::addressof(m));
    assert(lk.owns_lock() == false);
    }
}
