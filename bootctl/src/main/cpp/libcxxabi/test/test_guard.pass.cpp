//===----------------------------- test_guard.cpp -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "cxxabi.h"

#include <cassert>

#ifndef _LIBCXXABI_HAS_NO_THREADS
#include <thread>
#endif

// Ensure that we initialize each variable once and only once.
namespace test1 {
    static int run_count = 0;
    int increment() {
        ++run_count;
        return 0;
    }
    void helper() {
        static int a = increment();
        ((void)a);
    }
    void test() {
        static int a = increment(); ((void)a);
        assert(run_count == 1);
        static int b = increment(); ((void)b);
        assert(run_count == 2);
        helper();
        assert(run_count == 3);
        helper();
        assert(run_count == 3);
    }
}

// When initialization fails, ensure that we try to initialize it again next
// time.
namespace test2 {
#ifndef LIBCXXABI_HAS_NO_EXCEPTIONS
    static int run_count = 0;
    int increment() {
        ++run_count;
        throw 0;
    }
    void helper() {
        try {
            static int a = increment();
            assert(false);
            ((void)a);
        } catch (...) {}
    }
    void test() {
        helper();
        assert(run_count == 1);
        helper();
        assert(run_count == 2);
    }
#else
   void test() {}
#endif
}

// Check that we can initialize a second value while initializing a first.
namespace test3 {
    int zero() {
        return 0;
    }

    int one() {
        static int b = zero(); ((void)b);
        return 0;
    }

    void test() {
        static int a = one(); ((void)a);
    }
}

#ifndef _LIBCXXABI_HAS_NO_THREADS
// A simple thread test of two threads racing to initialize a variable. This
// isn't guaranteed to catch any particular threading problems.
namespace test4 {
    static int run_count = 0;
    int increment() {
        ++run_count;
        return 0;
    }

    void helper() {
        static int a = increment(); ((void)a);
    }

    void test() {
        std::thread t1(helper), t2(helper);
        t1.join();
        t2.join();
        assert(run_count == 1);
    }
}

// Check that we don't re-initialize a static variable even when it's
// encountered from two different threads.
namespace test5 {
    static int run_count = 0;
    int zero() {
        ++run_count;
        return 0;
    }

    int one() {
        static int b = zero(); ((void)b);
        return 0;
    }

    void another_helper() {
        static int a = one(); ((void)a);
    }

    void helper() {
        static int a = one(); ((void)a);
        std::thread t(another_helper);
        t.join();
    }

    void test() {
        std::thread t(helper);
        t.join();
        assert(run_count == 1);
    }
}
#endif /* _LIBCXXABI_HAS_NO_THREADS */

int main()
{
    test1::test();
    test2::test();
    test3::test();
#ifndef _LIBCXXABI_HAS_NO_THREADS
    test4::test();
    test5::test();
#endif
}
