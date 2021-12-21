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

// class shared_timed_mutex;

#include <shared_mutex>

#include <atomic>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <cassert>

std::shared_timed_mutex m;

const int total_readers = 2;
std::atomic<int> readers_started(0);
std::atomic<int> readers_finished(0);

// Wait for the readers to start then try and acquire the write lock.
void writer_one() {
  while (readers_started != total_readers) {}
  bool b = m.try_lock_for(std::chrono::milliseconds(500));
  assert(b == false);
}

void blocked_reader() {
  ++readers_started;
  // Wait until writer_one is waiting for the write lock.
  while (m.try_lock_shared()) {
    m.unlock_shared();
  }
  // Attempt to get the read lock. writer_one should be blocking us because
  // writer_one is blocked by main.
  m.lock_shared();
  ++readers_finished;
  m.unlock_shared();
}

int main()
{
  typedef std::chrono::steady_clock Clock;

  m.lock_shared();
  std::thread t1(writer_one);
  // create some readers
  std::thread t2(blocked_reader);
  std::thread t3(blocked_reader);
  // Kill the test after 10 seconds if it hasn't completed.
  auto end_point = Clock::now() + std::chrono::seconds(10);
  while (readers_finished != total_readers && Clock::now() < end_point) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  assert(readers_finished == total_readers);
  m.unlock_shared();
  t1.join();
  t2.join();
  t3.join();
}
