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

// <thread>

// class thread::id

// bool operator< (thread::id x, thread::id y);
// bool operator<=(thread::id x, thread::id y);
// bool operator> (thread::id x, thread::id y);
// bool operator>=(thread::id x, thread::id y);

#include <thread>
#include <cassert>

int main()
{
    std::thread::id id0;
    std::thread::id id1;
    std::thread::id id2 = std::this_thread::get_id();
    assert(!(id0 <  id1));
    assert( (id0 <= id1));
    assert(!(id0 >  id1));
    assert( (id0 >= id1));
    assert(!(id0 == id2));
    if (id0 < id2) {
      assert( (id0 <= id2));
      assert(!(id0 >  id2));
      assert(!(id0 >= id2));
    } else {
      assert(!(id0 <= id2));
      assert( (id0 >  id2));
      assert( (id0 >= id2));
    }
}
