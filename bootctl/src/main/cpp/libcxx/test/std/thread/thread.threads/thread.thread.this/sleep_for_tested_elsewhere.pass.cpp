// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <thread>

// template <class Rep, class Period>
//   void sleep_for(const chrono::duration<Rep, Period>& rel_time);

// The std::this_thread::sleep_for test requires POSIX specific headers and
// is therefore non-standard. For this reason the test lives under the 'libcxx'
// subdirectory.

int main()
{
}
