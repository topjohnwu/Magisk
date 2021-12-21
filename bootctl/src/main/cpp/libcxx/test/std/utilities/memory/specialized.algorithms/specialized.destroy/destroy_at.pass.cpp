//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14

// <memory>

// template <class _Tp>
// void destroy_at(_Tp*);

#include <memory>
#include <cstdlib>
#include <cassert>

struct Counted {
  static int count;
  static void reset() { count = 0; }
  Counted() { ++count; }
  Counted(Counted const&) { ++count; }
  ~Counted() { --count; }
  friend void operator&(Counted) = delete;
};
int Counted::count = 0;

struct VCounted {
  static int count;
  static void reset() { count = 0; }
  VCounted() { ++count; }
  VCounted(VCounted const&) { ++count; }
  virtual ~VCounted() { --count; }
  friend void operator&(VCounted) = delete;
};
int VCounted::count = 0;

struct DCounted : VCounted {
    friend void operator&(DCounted) = delete;
};

int main()
{
    {
    void* mem1 = std::malloc(sizeof(Counted));
    void* mem2 = std::malloc(sizeof(Counted));
    assert(mem1 && mem2);
    assert(Counted::count == 0);
    Counted* ptr1 = ::new(mem1) Counted();
    Counted* ptr2 = ::new(mem2) Counted();
    assert(Counted::count == 2);
    std::destroy_at(ptr1);
    assert(Counted::count == 1);
    std::destroy_at(ptr2);
    assert(Counted::count == 0);
    std::free(mem1);
    std::free(mem2);
    }
    {
    void* mem1 = std::malloc(sizeof(DCounted));
    void* mem2 = std::malloc(sizeof(DCounted));
    assert(mem1 && mem2);
    assert(DCounted::count == 0);
    DCounted* ptr1 = ::new(mem1) DCounted();
    DCounted* ptr2 = ::new(mem2) DCounted();
    assert(DCounted::count == 2);
    assert(VCounted::count == 2);
    std::destroy_at(ptr1);
    assert(VCounted::count == 1);
    std::destroy_at(ptr2);
    assert(VCounted::count == 0);
    std::free(mem1);
    std::free(mem2);
    }
}
