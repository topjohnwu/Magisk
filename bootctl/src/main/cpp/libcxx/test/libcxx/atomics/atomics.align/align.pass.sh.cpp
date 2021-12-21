//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// UNSUPPORTED: libcpp-has-no-threads, c++98, c++03
// REQUIRES: libatomic
// RUN: %build -latomic
// RUN: %run
//
// GCC currently fails because it needs -fabi-version=6 to fix mangling of
// std::atomic when used with __attribute__((vector(X))).
// XFAIL: gcc

// <atomic>

// Verify that the content of atomic<T> is properly aligned if the type is
// lock-free. This can't be observed through the atomic<T> API. It is
// nonetheless required for correctness of the implementation: lock-free implies
// that ISA instructions are used, and these instructions assume "suitable
// alignment". Supported architectures all require natural alignment for
// lock-freedom (e.g. load-linked / store-conditional, or cmpxchg).

#include <atomic>
#include <cassert>

template <typename T> struct atomic_test : public std::__atomic_base<T> {
  atomic_test() {
    if (this->is_lock_free())
      assert(alignof(this->__a_) >= sizeof(this->__a_) &&
             "expected natural alignment for lock-free type");
  }
};

int main() {

// structs and unions can't be defined in the template invocation.
// Work around this with a typedef.
#define CHECK_ALIGNMENT(T)                                                     \
  do {                                                                         \
    typedef T type;                                                            \
    atomic_test<type> t;                                                       \
  } while (0)

  CHECK_ALIGNMENT(bool);
  CHECK_ALIGNMENT(char);
  CHECK_ALIGNMENT(signed char);
  CHECK_ALIGNMENT(unsigned char);
  CHECK_ALIGNMENT(char16_t);
  CHECK_ALIGNMENT(char32_t);
  CHECK_ALIGNMENT(wchar_t);
  CHECK_ALIGNMENT(short);
  CHECK_ALIGNMENT(unsigned short);
  CHECK_ALIGNMENT(int);
  CHECK_ALIGNMENT(unsigned int);
  CHECK_ALIGNMENT(long);
  CHECK_ALIGNMENT(unsigned long);
  CHECK_ALIGNMENT(long long);
  CHECK_ALIGNMENT(unsigned long long);
  CHECK_ALIGNMENT(std::nullptr_t);
  CHECK_ALIGNMENT(void *);
  CHECK_ALIGNMENT(float);
  CHECK_ALIGNMENT(double);
  CHECK_ALIGNMENT(long double);
  CHECK_ALIGNMENT(int __attribute__((vector_size(1 * sizeof(int)))));
  CHECK_ALIGNMENT(int __attribute__((vector_size(2 * sizeof(int)))));
  CHECK_ALIGNMENT(int __attribute__((vector_size(4 * sizeof(int)))));
  CHECK_ALIGNMENT(int __attribute__((vector_size(16 * sizeof(int)))));
  CHECK_ALIGNMENT(int __attribute__((vector_size(32 * sizeof(int)))));
  CHECK_ALIGNMENT(float __attribute__((vector_size(1 * sizeof(float)))));
  CHECK_ALIGNMENT(float __attribute__((vector_size(2 * sizeof(float)))));
  CHECK_ALIGNMENT(float __attribute__((vector_size(4 * sizeof(float)))));
  CHECK_ALIGNMENT(float __attribute__((vector_size(16 * sizeof(float)))));
  CHECK_ALIGNMENT(float __attribute__((vector_size(32 * sizeof(float)))));
  CHECK_ALIGNMENT(double __attribute__((vector_size(1 * sizeof(double)))));
  CHECK_ALIGNMENT(double __attribute__((vector_size(2 * sizeof(double)))));
  CHECK_ALIGNMENT(double __attribute__((vector_size(4 * sizeof(double)))));
  CHECK_ALIGNMENT(double __attribute__((vector_size(16 * sizeof(double)))));
  CHECK_ALIGNMENT(double __attribute__((vector_size(32 * sizeof(double)))));
  CHECK_ALIGNMENT(struct Empty {});
  CHECK_ALIGNMENT(struct OneInt { int i; });
  CHECK_ALIGNMENT(struct IntArr2 { int i[2]; });
  CHECK_ALIGNMENT(struct LLIArr2 { long long int i[2]; });
  CHECK_ALIGNMENT(struct LLIArr4 { long long int i[4]; });
  CHECK_ALIGNMENT(struct LLIArr8 { long long int i[8]; });
  CHECK_ALIGNMENT(struct LLIArr16 { long long int i[16]; });
  CHECK_ALIGNMENT(struct Padding { char c; /* padding */ long long int i; });
  CHECK_ALIGNMENT(union IntFloat { int i; float f; });
}
