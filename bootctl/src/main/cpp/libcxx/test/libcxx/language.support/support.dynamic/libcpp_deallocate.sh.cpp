//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test libc++'s implementation of align_val_t, and the relevant new/delete
// overloads in all dialects when -faligned-allocation is present.

// Libc++ defers to the underlying MSVC library to provide the new/delete
// definitions, which does not yet provide aligned allocation
// XFAIL: LIBCXX-WINDOWS-FIXME

// The dylibs shipped before macosx10.14 do not contain the aligned allocation
// functions, so trying to force using those with -faligned-allocation results
// in a link error.
// XFAIL: with_system_cxx_lib=macosx10.13
// XFAIL: with_system_cxx_lib=macosx10.12
// XFAIL: with_system_cxx_lib=macosx10.11
// XFAIL: with_system_cxx_lib=macosx10.10
// XFAIL: with_system_cxx_lib=macosx10.9
// XFAIL: with_system_cxx_lib=macosx10.8
// XFAIL: with_system_cxx_lib=macosx10.7

// The test will fail on deployment targets that do not support sized deallocation.
// XFAIL: availability=macosx10.11
// XFAIL: availability=macosx10.10
// XFAIL: availability=macosx10.9
// XFAIL: availability=macosx10.8
// XFAIL: availability=macosx10.7

// XFAIL: sanitizer-new-delete, ubsan

// GCC doesn't support the aligned-allocation flags.
// XFAIL: gcc

// RUN: %build -faligned-allocation -fsized-deallocation
// RUN: %run
// RUN: %build -faligned-allocation -fno-sized-deallocation -DNO_SIZE
// RUN: %run
// RUN: %build -fno-aligned-allocation -fsized-deallocation -DNO_ALIGN
// RUN: %run
// RUN: %build -fno-aligned-allocation -fno-sized-deallocation -DNO_ALIGN -DNO_SIZE
// RUN: %run

#include <new>
#include <typeinfo>
#include <string>
#include <cassert>

#include "test_macros.h"

struct alloc_stats {
  alloc_stats() { reset(); }

  int aligned_sized_called;
  int aligned_called;
  int sized_called;
  int plain_called;
  int last_size;
  int last_align;

  void reset() {
    aligned_sized_called = aligned_called = sized_called = plain_called = 0;
    last_align = last_size = -1;
  }

  bool expect_plain() const {
    assert(aligned_sized_called == 0);
    assert(aligned_called == 0);
    assert(sized_called == 0);
    assert(last_size == -1);
    assert(last_align == -1);
    return plain_called == 1;
  }

  bool expect_size(int n) const {
    assert(plain_called == 0);
    assert(aligned_sized_called == 0);
    assert(aligned_called == 0);
    assert(last_size == n);
    assert(last_align == -1);
    return sized_called == 1;
  }

  bool expect_align(int a) const {
    assert(plain_called == 0);
    assert(aligned_sized_called == 0);
    assert(sized_called == 0);
    assert(last_size == -1);
    assert(last_align == a);
    return aligned_called == 1;
  }

  bool expect_size_align(int n, int a) const {
    assert(plain_called == 0);
    assert(sized_called == 0);
    assert(aligned_called == 0);
    assert(last_size == n);
    assert(last_align == a);
    return aligned_sized_called == 1;
  }
};
alloc_stats stats;

void operator delete(void* p)TEST_NOEXCEPT {
  ::free(p);
  stats.plain_called++;
  stats.last_size = stats.last_align = -1;
}

#ifndef NO_SIZE
void operator delete(void* p, size_t n)TEST_NOEXCEPT {
  ::free(p);
  stats.sized_called++;
  stats.last_size = n;
  stats.last_align = -1;
}
#endif

#ifndef NO_ALIGN
void operator delete(void* p, std::align_val_t a)TEST_NOEXCEPT {
  ::free(p);
  stats.aligned_called++;
  stats.last_align = static_cast<int>(a);
  stats.last_size = -1;
}

void operator delete(void* p, size_t n, std::align_val_t a)TEST_NOEXCEPT {
  ::free(p);
  stats.aligned_sized_called++;
  stats.last_align = static_cast<int>(a);
  stats.last_size = n;
}
#endif

void test_libcpp_dealloc() {
  void* p = nullptr;
  size_t over_align_val = TEST_ALIGNOF(std::max_align_t) * 2;
  size_t under_align_val = TEST_ALIGNOF(int);
  size_t with_size_val = 2;

  {
    std::__libcpp_deallocate_unsized(p, under_align_val);
    assert(stats.expect_plain());
  }
  stats.reset();

#if defined(NO_SIZE) && defined(NO_ALIGN)
  {
    std::__libcpp_deallocate(p, with_size_val, over_align_val);
    assert(stats.expect_plain());
  }
  stats.reset();
#elif defined(NO_SIZE)
  {
    std::__libcpp_deallocate(p, with_size_val, over_align_val);
    assert(stats.expect_align(over_align_val));
  }
  stats.reset();
#elif defined(NO_ALIGN)
  {
    std::__libcpp_deallocate(p, with_size_val, over_align_val);
    assert(stats.expect_size(with_size_val));
  }
  stats.reset();
#else
  {
    std::__libcpp_deallocate(p, with_size_val, over_align_val);
    assert(stats.expect_size_align(with_size_val, over_align_val));
  }
  stats.reset();
  {
    std::__libcpp_deallocate_unsized(p, over_align_val);
    assert(stats.expect_align(over_align_val));
  }
  stats.reset();
  {
    std::__libcpp_deallocate(p, with_size_val, under_align_val);
    assert(stats.expect_size(with_size_val));
  }
  stats.reset();
#endif
}

struct TEST_ALIGNAS(128) AlignedType {
  AlignedType() : elem(0) {}
  TEST_ALIGNAS(128) char elem;
};

void test_allocator_and_new_match() {
  stats.reset();
#if defined(NO_SIZE) && defined(NO_ALIGN)
  {
    int* x = new int(42);
    delete x;
    assert(stats.expect_plain());
  }
  stats.reset();
  {
    AlignedType* a = new AlignedType();
    delete a;
    assert(stats.expect_plain());
  }
  stats.reset();
#elif defined(NO_SIZE)
  stats.reset();
#if TEST_STD_VER >= 11
  {
    int* x = new int(42);
    delete x;
    assert(stats.expect_plain());
  }
#endif
  stats.reset();
  {
    AlignedType* a = new AlignedType();
    delete a;
    assert(stats.expect_align(TEST_ALIGNOF(AlignedType)));
  }
  stats.reset();
#elif defined(NO_ALIGN)
  stats.reset();
  {
    int* x = new int(42);
    delete x;
    assert(stats.expect_size(sizeof(int)));
  }
  stats.reset();
  {
    AlignedType* a = new AlignedType();
    delete a;
    assert(stats.expect_size(sizeof(AlignedType)));
  }
  stats.reset();
#else
  stats.reset();
  {
    int* x = new int(42);
    delete x;
    assert(stats.expect_size(sizeof(int)));
  }
  stats.reset();
  {
    AlignedType* a = new AlignedType();
    delete a;
    assert(stats.expect_size_align(sizeof(AlignedType),
                                   TEST_ALIGNOF(AlignedType)));
  }
  stats.reset();
#endif
}

int main() {
  test_libcpp_dealloc();
  test_allocator_and_new_match();
}
