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

// unique_lock(unique_lock const&) = delete;

#include <mutex>
#include <cassert>

int main()
{
    {
    typedef std::mutex M;
    M m;
    std::unique_lock<M> lk0(m);
    std::unique_lock<M> lk = lk0;
    assert(lk.mutex() == &m);
    assert(lk.owns_lock() == true);
    assert(lk0.mutex() == nullptr);
    assert(lk0.owns_lock() == false);
    }
}
