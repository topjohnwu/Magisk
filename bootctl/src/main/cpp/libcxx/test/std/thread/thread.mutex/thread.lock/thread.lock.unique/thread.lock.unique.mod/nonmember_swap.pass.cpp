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

// <mutex>

// template <class Mutex> class unique_lock;

// template <class Mutex>
//   void swap(unique_lock<Mutex>& x, unique_lock<Mutex>& y);

#include <mutex>
#include <cassert>

struct mutex
{
    void lock() {}
    void unlock() {}
};

mutex m;

int main()
{
    std::unique_lock<mutex> lk1(m);
    std::unique_lock<mutex> lk2;
    swap(lk1, lk2);
    assert(lk1.mutex() == nullptr);
    assert(lk1.owns_lock() == false);
    assert(lk2.mutex() == &m);
    assert(lk2.owns_lock() == true);
}
