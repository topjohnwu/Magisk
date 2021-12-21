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
//
// <memory>
//
// class shared_ptr
//
// This test attempts to create a race condition surrounding use_count()
// with the hope that TSAN will diagnose it.

#include <memory>
#include <atomic>
#include <thread>
#include <cassert>

typedef std::shared_ptr<int> Ptr;
typedef std::weak_ptr<int> WeakPtr;

std::atomic_bool Start;
std::atomic_bool KeepRunning;

struct TestRunner {
    TestRunner(Ptr xx) : x(xx) {}
    void operator()() {
        while (Start == false) {}
        while (KeepRunning) {
            // loop to prevent always checking the atomic.
            for (int i=0; i < 100000; ++i) {
                Ptr x2 = x; // increment shared count
                WeakPtr x3 = x; // increment weak count
                Ptr x4 = x3.lock(); // increment shared count via lock
                WeakPtr x5 = x3; // increment weak count
            }
        }
    }
    Ptr x;
};

void run_test(Ptr p) {
    Start = false;
    KeepRunning = true;
    assert(p.use_count() == 2);
    TestRunner r(p);
    assert(p.use_count() == 3);
    std::thread t1(r); // Start the test thread.
    assert(p.use_count() == 4);
    Start = true;
    // Run until we witness 25 use count changes via both
    // shared and weak pointer methods.
    WeakPtr w = p;
    int shared_changes_count = 0;
    int weak_changes_count = 0;
    while (shared_changes_count < 25 && weak_changes_count < 25) {
        // check use_count on the shared_ptr
       int last = p.use_count();
       int new_val = p.use_count();
       assert(last >= 4);
       assert(new_val >= 4);
       if (last != new_val) ++shared_changes_count;
       // Check use_count on the weak_ptr
       last = w.use_count();
       new_val = w.use_count();
       assert(last >= 4);
       assert(new_val >= 4);
       if (last != new_val) ++weak_changes_count;
    }
    // kill the test thread.
    KeepRunning = false;
    t1.join();
    assert(p.use_count() == 3);
}

int main() {
  {
    // Test with out-of-place shared_count.
    Ptr p(new int(42));
    run_test(p);
    assert(p.use_count() == 1);
  }
  {
    // Test with in-place shared_count.
    int val = 42;
    Ptr p = std::make_shared<int>(val);
    run_test(p);
    assert(p.use_count() == 1);
  }
}
