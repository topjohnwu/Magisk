//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: libcpp-no-exceptions
// UNSUPPORTED: libcpp-has-no-threads

// <condition_variable>

// class condition_variable_any;

// RUN: %build
// RUN: %run 1
// RUN: %run 2
// RUN: %run 3
// RUN: %run 4
// RUN: %run 5
// RUN: %run 6

// -----------------------------------------------------------------------------
// Overview
//   Check that std::terminate is called if wait(...) fails to meet its post
//   conditions. This can happen when reacquiring the mutex throws
//   an exception.
//
//  The following methods are tested within this file
//   1.  void wait(Lock& lock);
//   2.  void wait(Lock& lock, Pred);
//   3.  void wait_for(Lock& lock, Duration);
//   4.  void wait_for(Lock& lock, Duration, Pred);
//   5.  void wait_until(Lock& lock, TimePoint);
//   6.  void wait_until(Lock& lock, TimePoint, Pred);
//
// Plan
//   1 Create a mutex type, 'ThrowingMutex', that throws when the lock is acquired
//     for the *second* time.
//
//   2 Replace the terminate handler with one that exits with a '0' exit code.
//
//   3 Create a 'condition_variable_any' object 'cv' and a 'ThrowingMutex'
//     object 'm' and lock 'm'.
//
//   4 Start a thread 'T2' that will notify 'cv' once 'm' has been unlocked.
//
//   5 From the main thread call the specified wait method on 'cv' with 'm'.
//     When 'T2' notifies 'cv' and the wait method attempts to re-lock
//    'm' an exception will be thrown from 'm.lock()'.
//
//   6 Check that control flow does not return from the wait method and that
//     terminate is called (If the program exits with a 0 exit code we know
//     that terminate has been called)


#include <condition_variable>
#include <atomic>
#include <thread>
#include <chrono>
#include <string>
#include <cstdlib>
#include <cassert>

void my_terminate() {
  std::_Exit(0); // Use _Exit to prevent cleanup from taking place.
}

// The predicate used in the cv.wait calls.
bool pred = false;
bool pred_function() {
  return pred == true;
}

class ThrowingMutex
{
  std::atomic_bool locked;
  unsigned state = 0;
  ThrowingMutex(const ThrowingMutex&) = delete;
  ThrowingMutex& operator=(const ThrowingMutex&) = delete;
public:
  ThrowingMutex() {
    locked = false;
  }
  ~ThrowingMutex() = default;

  void lock() {
    locked = true;
    if (++state == 2) {
      assert(pred); // Check that we actually waited until we were signaled.
      throw 1;  // this throw should end up calling terminate()
    }
  }

  void unlock() { locked = false; }
  bool isLocked() const { return locked == true; }
};

ThrowingMutex mut;
std::condition_variable_any cv;

void signal_me() {
  while (mut.isLocked()) {} // wait until T1 releases mut inside the cv.wait call.
  pred = true;
  cv.notify_one();
}

typedef std::chrono::system_clock Clock;
typedef std::chrono::milliseconds MS;

int main(int argc, char** argv) {
  assert(argc == 2);
  int id = std::stoi(argv[1]);
  assert(id >= 1 && id <= 6);
  std::set_terminate(my_terminate); // set terminate after std::stoi because it can throw.
  MS wait(250);
  try {
    mut.lock();
    assert(pred == false);
    std::thread(signal_me).detach();
    switch (id) {
      case 1: cv.wait(mut); break;
      case 2: cv.wait(mut, pred_function); break;
      case 3: cv.wait_for(mut, wait); break;
      case 4: cv.wait_for(mut, wait, pred_function); break;
      case 5: cv.wait_until(mut, Clock::now() + wait); break;
      case 6: cv.wait_until(mut, Clock::now() + wait, pred_function); break;
      default: assert(false);
    }
  } catch (...) {}
  assert(false);
}
