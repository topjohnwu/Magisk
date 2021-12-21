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
// UNSUPPORTED: c++98, c++03, c++11, c++14

// <mutex>

// template <class ...Mutex> class scoped_lock;

// scoped_lock(adopt_lock_t, Mutex&...);

#include <mutex>
#include <cassert>
#include "test_macros.h"

struct TestMutex {
    bool locked = false;
    TestMutex() = default;

    void lock() { assert(!locked); locked = true; }
    bool try_lock() { if (locked) return false; locked = true; return true; }
    void unlock() { assert(locked); locked = false; }

    TestMutex(TestMutex const&) = delete;
    TestMutex& operator=(TestMutex const&) = delete;
};

int main()
{
    {
        using LG = std::scoped_lock<>;
        LG lg(std::adopt_lock);
    }
    {
        TestMutex m1;
        using LG = std::scoped_lock<TestMutex>;
        m1.lock();
        {
            LG lg(std::adopt_lock, m1);
            assert(m1.locked);
        }
        assert(!m1.locked);
    }
    {
        TestMutex m1, m2;
        using LG = std::scoped_lock<TestMutex, TestMutex>;
        m1.lock(); m2.lock();
        {
            LG lg(std::adopt_lock, m1, m2);
            assert(m1.locked && m2.locked);
        }
        assert(!m1.locked && !m2.locked);
    }
    {
        TestMutex m1, m2, m3;
        using LG = std::scoped_lock<TestMutex, TestMutex, TestMutex>;
        m1.lock(); m2.lock(); m3.lock();
        {
            LG lg(std::adopt_lock, m1, m2, m3);
            assert(m1.locked && m2.locked && m3.locked);
        }
        assert(!m1.locked && !m2.locked && !m3.locked);
    }

}
