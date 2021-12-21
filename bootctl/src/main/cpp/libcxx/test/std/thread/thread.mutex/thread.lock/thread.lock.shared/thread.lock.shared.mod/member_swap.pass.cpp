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

// void swap(shared_lock& u) noexcept;

#include <shared_mutex>
#include <cassert>

struct mutex
{
    void lock_shared() {}
    void unlock_shared() {}
};

mutex m;

int main()
{
    std::shared_lock<mutex> lk1(m);
    std::shared_lock<mutex> lk2;
    lk1.swap(lk2);
    assert(lk1.mutex() == nullptr);
    assert(lk1.owns_lock() == false);
    assert(lk2.mutex() == &m);
    assert(lk2.owns_lock() == true);
    static_assert(noexcept(lk1.swap(lk2)), "member swap must be noexcept");
}
