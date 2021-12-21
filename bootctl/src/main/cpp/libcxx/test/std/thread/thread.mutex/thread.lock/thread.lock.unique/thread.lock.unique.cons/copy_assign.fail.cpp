//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <mutex>

// template <class Mutex> class unique_lock;

// unique_lock& operator=(unique_lock const&) = delete;

#include <mutex>
#include <cassert>

int main()
{
    {
    typedef std::mutex M;
    M m0;
    M m1;
    std::unique_lock<M> lk0(m0);
    std::unique_lock<M> lk1(m1);
    lk1 = lk0;
    assert(lk1.mutex() == &m0);
    assert(lk1.owns_lock() == true);
    assert(lk0.mutex() == nullptr);
    assert(lk0.owns_lock() == false);
    }
}
