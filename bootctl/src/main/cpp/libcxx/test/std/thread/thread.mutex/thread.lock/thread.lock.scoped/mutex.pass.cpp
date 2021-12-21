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

// explicit scoped_lock(mutex_type& m);

#include <mutex>
#include <cassert>
#include "test_macros.h"

struct TestMutex {
    bool locked = false;
    TestMutex() = default;
    ~TestMutex() { assert(!locked); }

    void lock() { assert(!locked); locked = true; }
    bool try_lock() { if (locked) return false; locked = true; return true; }
    void unlock() { assert(locked); locked = false; }

    TestMutex(TestMutex const&) = delete;
    TestMutex& operator=(TestMutex const&) = delete;
};

#if !defined(TEST_HAS_NO_EXCEPTIONS)
struct TestMutexThrows {
    bool locked = false;
    bool throws_on_lock = false;

    TestMutexThrows() = default;
    ~TestMutexThrows() { assert(!locked); }

    void lock() {
        assert(!locked);
        if (throws_on_lock) {
            throw 42;
        }
        locked = true;
    }

    bool try_lock() {
        if (locked) return false;
        lock();
        return true;
    }

    void unlock() { assert(locked); locked = false; }

    TestMutexThrows(TestMutexThrows const&) = delete;
    TestMutexThrows& operator=(TestMutexThrows const&) = delete;
};
#endif // !defined(TEST_HAS_NO_EXCEPTIONS)

int main()
{
    {
        using LG = std::scoped_lock<>;
        LG lg;
    }
    {
        using LG = std::scoped_lock<TestMutex>;
        TestMutex m1;
        {
            LG lg(m1);
            assert(m1.locked);
        }
        assert(!m1.locked);
    }
    {
        using LG = std::scoped_lock<TestMutex, TestMutex>;
        TestMutex m1, m2;
        {
            LG lg(m1, m2);
            assert(m1.locked && m2.locked);
        }
        assert(!m1.locked && !m2.locked);
    }
    {
        using LG = std::scoped_lock<TestMutex, TestMutex, TestMutex>;
        TestMutex m1, m2, m3;
        {
            LG lg(m1, m2, m3);
            assert(m1.locked && m2.locked && m3.locked);
        }
        assert(!m1.locked && !m2.locked && !m3.locked);
    }
#if !defined(TEST_HAS_NO_EXCEPTIONS)
    {
        using MT = TestMutexThrows;
        using LG = std::scoped_lock<MT>;
        MT m1;
        m1.throws_on_lock = true;
        try {
            LG lg(m1);
            assert(false);
        } catch (int) {}
        assert(!m1.locked);
    }
    {
        using MT = TestMutexThrows;
        using LG = std::scoped_lock<MT, MT>;
        MT m1, m2;
        m1.throws_on_lock = true;
        try {
            LG lg(m1, m2);
            assert(false);
        } catch (int) {}
        assert(!m1.locked && !m2.locked);
    }
    {
        using MT = TestMutexThrows;
        using LG = std::scoped_lock<MT, MT, MT>;
        MT m1, m2, m3;
        m2.throws_on_lock = true;
        try {
            LG lg(m1, m2, m3);
            assert(false);
        } catch (int) {}
        assert(!m1.locked && !m2.locked && !m3.locked);
    }
#endif

#ifdef __cpp_deduction_guides
    {
    TestMutex m1, m2, m3;
        {
        std::scoped_lock sl{};
        static_assert((std::is_same<decltype(sl), std::scoped_lock<>>::value), "" );
        }
        {
        std::scoped_lock sl{m1};
        static_assert((std::is_same<decltype(sl), std::scoped_lock<decltype(m1)>>::value), "" );
        }
        {
        std::scoped_lock sl{m1, m2};
        static_assert((std::is_same<decltype(sl), std::scoped_lock<decltype(m1), decltype(m2)>>::value), "" );
        }
        {
        std::scoped_lock sl{m1, m2, m3};
        static_assert((std::is_same<decltype(sl), std::scoped_lock<decltype(m1), decltype(m2), decltype(m3)>>::value), "" );
        }
    }
#endif
}
