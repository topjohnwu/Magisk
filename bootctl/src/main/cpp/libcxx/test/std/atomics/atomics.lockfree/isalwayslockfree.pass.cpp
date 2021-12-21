//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// UNSUPPORTED: libcpp-has-no-threads, c++98, c++03, c++11, c++14

// <atomic>

// static constexpr bool is_always_lock_free;

#include <atomic>
#include <cassert>

#if !defined(__cpp_lib_atomic_is_always_lock_free)
# error Feature test macro missing.
#endif

template <typename T> void checkAlwaysLockFree() {
  if (std::atomic<T>::is_always_lock_free)
    assert(std::atomic<T>().is_lock_free());
}

// FIXME: This separate test is needed to work around llvm.org/PR31864
// which causes ATOMIC_LLONG_LOCK_FREE to be defined as '1' in 32-bit builds
// even though __atomic_always_lock_free returns true for the same type.
constexpr bool NeedWorkaroundForPR31864 =
#if defined(__clang__)
(sizeof(void*) == 4); // Needed on 32 bit builds
#else
false;
#endif

template <bool Disable = NeedWorkaroundForPR31864,
  std::enable_if_t<!Disable>* = nullptr,
  class LLong = long long,
  class ULLong = unsigned long long>
void checkLongLongTypes() {
  static_assert(std::atomic<LLong>::is_always_lock_free == (2 == ATOMIC_LLONG_LOCK_FREE));
  static_assert(std::atomic<ULLong>::is_always_lock_free == (2 == ATOMIC_LLONG_LOCK_FREE));
}

// Used to make the calls to __atomic_always_lock_free dependent on a template
// parameter.
template <class T> constexpr size_t getSizeOf() { return sizeof(T); }

template <bool Enable = NeedWorkaroundForPR31864,
  std::enable_if_t<Enable>* = nullptr,
  class LLong = long long,
  class ULLong = unsigned long long>
void checkLongLongTypes() {
  constexpr bool ExpectLockFree = __atomic_always_lock_free(getSizeOf<LLong>(), 0);
  static_assert(std::atomic<LLong>::is_always_lock_free == ExpectLockFree, "");
  static_assert(std::atomic<ULLong>::is_always_lock_free == ExpectLockFree, "");
  static_assert((0 != ATOMIC_LLONG_LOCK_FREE) == ExpectLockFree, "");
}

void run()
{
// structs and unions can't be defined in the template invocation.
// Work around this with a typedef.
#define CHECK_ALWAYS_LOCK_FREE(T)                                              \
  do {                                                                         \
    typedef T type;                                                            \
    checkAlwaysLockFree<type>();                                               \
  } while (0)

    CHECK_ALWAYS_LOCK_FREE(bool);
    CHECK_ALWAYS_LOCK_FREE(char);
    CHECK_ALWAYS_LOCK_FREE(signed char);
    CHECK_ALWAYS_LOCK_FREE(unsigned char);
    CHECK_ALWAYS_LOCK_FREE(char16_t);
    CHECK_ALWAYS_LOCK_FREE(char32_t);
    CHECK_ALWAYS_LOCK_FREE(wchar_t);
    CHECK_ALWAYS_LOCK_FREE(short);
    CHECK_ALWAYS_LOCK_FREE(unsigned short);
    CHECK_ALWAYS_LOCK_FREE(int);
    CHECK_ALWAYS_LOCK_FREE(unsigned int);
    CHECK_ALWAYS_LOCK_FREE(long);
    CHECK_ALWAYS_LOCK_FREE(unsigned long);
    CHECK_ALWAYS_LOCK_FREE(long long);
    CHECK_ALWAYS_LOCK_FREE(unsigned long long);
    CHECK_ALWAYS_LOCK_FREE(std::nullptr_t);
    CHECK_ALWAYS_LOCK_FREE(void*);
    CHECK_ALWAYS_LOCK_FREE(float);
    CHECK_ALWAYS_LOCK_FREE(double);
    CHECK_ALWAYS_LOCK_FREE(long double);
#if __has_attribute(vector_size) && defined(_LIBCPP_VERSION)
    CHECK_ALWAYS_LOCK_FREE(int __attribute__((vector_size(1 * sizeof(int)))));
    CHECK_ALWAYS_LOCK_FREE(int __attribute__((vector_size(2 * sizeof(int)))));
    CHECK_ALWAYS_LOCK_FREE(int __attribute__((vector_size(4 * sizeof(int)))));
    CHECK_ALWAYS_LOCK_FREE(int __attribute__((vector_size(16 * sizeof(int)))));
    CHECK_ALWAYS_LOCK_FREE(int __attribute__((vector_size(32 * sizeof(int)))));
    CHECK_ALWAYS_LOCK_FREE(float __attribute__((vector_size(1 * sizeof(float)))));
    CHECK_ALWAYS_LOCK_FREE(float __attribute__((vector_size(2 * sizeof(float)))));
    CHECK_ALWAYS_LOCK_FREE(float __attribute__((vector_size(4 * sizeof(float)))));
    CHECK_ALWAYS_LOCK_FREE(float __attribute__((vector_size(16 * sizeof(float)))));
    CHECK_ALWAYS_LOCK_FREE(float __attribute__((vector_size(32 * sizeof(float)))));
    CHECK_ALWAYS_LOCK_FREE(double __attribute__((vector_size(1 * sizeof(double)))));
    CHECK_ALWAYS_LOCK_FREE(double __attribute__((vector_size(2 * sizeof(double)))));
    CHECK_ALWAYS_LOCK_FREE(double __attribute__((vector_size(4 * sizeof(double)))));
    CHECK_ALWAYS_LOCK_FREE(double __attribute__((vector_size(16 * sizeof(double)))));
    CHECK_ALWAYS_LOCK_FREE(double __attribute__((vector_size(32 * sizeof(double)))));
#endif // __has_attribute(vector_size) && defined(_LIBCPP_VERSION)
    CHECK_ALWAYS_LOCK_FREE(struct Empty {});
    CHECK_ALWAYS_LOCK_FREE(struct OneInt { int i; });
    CHECK_ALWAYS_LOCK_FREE(struct IntArr2 { int i[2]; });
    CHECK_ALWAYS_LOCK_FREE(struct LLIArr2 { long long int i[2]; });
    CHECK_ALWAYS_LOCK_FREE(struct LLIArr4 { long long int i[4]; });
    CHECK_ALWAYS_LOCK_FREE(struct LLIArr8 { long long int i[8]; });
    CHECK_ALWAYS_LOCK_FREE(struct LLIArr16 { long long int i[16]; });
    CHECK_ALWAYS_LOCK_FREE(struct Padding { char c; /* padding */ long long int i; });
    CHECK_ALWAYS_LOCK_FREE(union IntFloat { int i; float f; });

    // C macro and static constexpr must be consistent.
    static_assert(std::atomic<bool>::is_always_lock_free == (2 == ATOMIC_BOOL_LOCK_FREE));
    static_assert(std::atomic<char>::is_always_lock_free == (2 == ATOMIC_CHAR_LOCK_FREE));
    static_assert(std::atomic<signed char>::is_always_lock_free == (2 == ATOMIC_CHAR_LOCK_FREE));
    static_assert(std::atomic<unsigned char>::is_always_lock_free == (2 == ATOMIC_CHAR_LOCK_FREE));
    static_assert(std::atomic<char16_t>::is_always_lock_free == (2 == ATOMIC_CHAR16_T_LOCK_FREE));
    static_assert(std::atomic<char32_t>::is_always_lock_free == (2 == ATOMIC_CHAR32_T_LOCK_FREE));
    static_assert(std::atomic<wchar_t>::is_always_lock_free == (2 == ATOMIC_WCHAR_T_LOCK_FREE));
    static_assert(std::atomic<short>::is_always_lock_free == (2 == ATOMIC_SHORT_LOCK_FREE));
    static_assert(std::atomic<unsigned short>::is_always_lock_free == (2 == ATOMIC_SHORT_LOCK_FREE));
    static_assert(std::atomic<int>::is_always_lock_free == (2 == ATOMIC_INT_LOCK_FREE));
    static_assert(std::atomic<unsigned int>::is_always_lock_free == (2 == ATOMIC_INT_LOCK_FREE));
    static_assert(std::atomic<long>::is_always_lock_free == (2 == ATOMIC_LONG_LOCK_FREE));
    static_assert(std::atomic<unsigned long>::is_always_lock_free == (2 == ATOMIC_LONG_LOCK_FREE));
    checkLongLongTypes();
    static_assert(std::atomic<void*>::is_always_lock_free == (2 == ATOMIC_POINTER_LOCK_FREE));
    static_assert(std::atomic<std::nullptr_t>::is_always_lock_free == (2 == ATOMIC_POINTER_LOCK_FREE));
}

int main() { run(); }
