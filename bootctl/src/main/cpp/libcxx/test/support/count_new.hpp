//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef COUNT_NEW_HPP
#define COUNT_NEW_HPP

# include <cstdlib>
# include <cassert>
# include <new>

#include "test_macros.h"

#if defined(TEST_HAS_SANITIZERS)
#define DISABLE_NEW_COUNT
#endif

namespace detail
{
   TEST_NORETURN
   inline void throw_bad_alloc_helper() {
#ifndef TEST_HAS_NO_EXCEPTIONS
       throw std::bad_alloc();
#else
       std::abort();
#endif
   }
}

class MemCounter
{
public:
    // Make MemCounter super hard to accidentally construct or copy.
    class MemCounterCtorArg_ {};
    explicit MemCounter(MemCounterCtorArg_) { reset(); }

private:
    MemCounter(MemCounter const &);
    MemCounter & operator=(MemCounter const &);

public:
    // All checks return true when disable_checking is enabled.
    static const bool disable_checking;

    // Disallow any allocations from occurring. Useful for testing that
    // code doesn't perform any allocations.
    bool disable_allocations;

    // number of allocations to throw after. Default (unsigned)-1. If
    // throw_after has the default value it will never be decremented.
    static const unsigned never_throw_value = static_cast<unsigned>(-1);
    unsigned throw_after;

    int outstanding_new;
    int new_called;
    int delete_called;
    int aligned_new_called;
    int aligned_delete_called;
    std::size_t last_new_size;
    std::size_t last_new_align;
    std::size_t last_delete_align;

    int outstanding_array_new;
    int new_array_called;
    int delete_array_called;
    int aligned_new_array_called;
    int aligned_delete_array_called;
    std::size_t last_new_array_size;
    std::size_t last_new_array_align;
    std::size_t last_delete_array_align;

public:
    void newCalled(std::size_t s)
    {
        assert(disable_allocations == false);
        assert(s);
        if (throw_after == 0) {
            throw_after = never_throw_value;
            detail::throw_bad_alloc_helper();
        } else if (throw_after != never_throw_value) {
            --throw_after;
        }
        ++new_called;
        ++outstanding_new;
        last_new_size = s;
    }

    void alignedNewCalled(std::size_t s, std::size_t a) {
      newCalled(s);
      ++aligned_new_called;
      last_new_align = a;
    }

    void deleteCalled(void * p)
    {
        assert(p);
        --outstanding_new;
        ++delete_called;
    }

    void alignedDeleteCalled(void *p, std::size_t a) {
      deleteCalled(p);
      ++aligned_delete_called;
      last_delete_align = a;
    }

    void newArrayCalled(std::size_t s)
    {
        assert(disable_allocations == false);
        assert(s);
        if (throw_after == 0) {
            throw_after = never_throw_value;
            detail::throw_bad_alloc_helper();
        } else {
            // don't decrement throw_after here. newCalled will end up doing that.
        }
        ++outstanding_array_new;
        ++new_array_called;
        last_new_array_size = s;
    }

    void alignedNewArrayCalled(std::size_t s, std::size_t a) {
      newArrayCalled(s);
      ++aligned_new_array_called;
      last_new_array_align = a;
    }

    void deleteArrayCalled(void * p)
    {
        assert(p);
        --outstanding_array_new;
        ++delete_array_called;
    }

    void alignedDeleteArrayCalled(void * p, std::size_t a) {
      deleteArrayCalled(p);
      ++aligned_delete_array_called;
      last_delete_array_align = a;
    }

    void disableAllocations()
    {
        disable_allocations = true;
    }

    void enableAllocations()
    {
        disable_allocations = false;
    }

    void reset()
    {
        disable_allocations = false;
        throw_after = never_throw_value;

        outstanding_new = 0;
        new_called = 0;
        delete_called = 0;
        aligned_new_called = 0;
        aligned_delete_called = 0;
        last_new_size = 0;
        last_new_align = 0;

        outstanding_array_new = 0;
        new_array_called = 0;
        delete_array_called = 0;
        aligned_new_array_called = 0;
        aligned_delete_array_called = 0;
        last_new_array_size = 0;
        last_new_array_align = 0;
    }

public:
    bool checkOutstandingNewEq(int n) const
    {
        return disable_checking || n == outstanding_new;
    }

    bool checkOutstandingNewNotEq(int n) const
    {
        return disable_checking || n != outstanding_new;
    }

    bool checkNewCalledEq(int n) const
    {
        return disable_checking || n == new_called;
    }

    bool checkNewCalledNotEq(int n) const
    {
        return disable_checking || n != new_called;
    }

    bool checkNewCalledGreaterThan(int n) const
    {
        return disable_checking || new_called > n;
    }

    bool checkDeleteCalledEq(int n) const
    {
        return disable_checking || n == delete_called;
    }

    bool checkDeleteCalledNotEq(int n) const
    {
        return disable_checking || n != delete_called;
    }

    bool checkAlignedNewCalledEq(int n) const
    {
        return disable_checking || n == aligned_new_called;
    }

    bool checkAlignedNewCalledNotEq(int n) const
    {
        return disable_checking || n != aligned_new_called;
    }

    bool checkAlignedNewCalledGreaterThan(int n) const
    {
        return disable_checking || aligned_new_called > n;
    }

    bool checkAlignedDeleteCalledEq(int n) const
    {
        return disable_checking || n == aligned_delete_called;
    }

    bool checkAlignedDeleteCalledNotEq(int n) const
    {
        return disable_checking || n != aligned_delete_called;
    }

    bool checkLastNewSizeEq(std::size_t n) const
    {
        return disable_checking || n == last_new_size;
    }

    bool checkLastNewSizeNotEq(std::size_t n) const
    {
        return disable_checking || n != last_new_size;
    }

    bool checkLastNewAlignEq(std::size_t n) const
    {
        return disable_checking || n == last_new_align;
    }

    bool checkLastNewAlignNotEq(std::size_t n) const
    {
        return disable_checking || n != last_new_align;
    }

    bool checkLastDeleteAlignEq(std::size_t n) const
    {
        return disable_checking || n == last_delete_align;
    }

    bool checkLastDeleteAlignNotEq(std::size_t n) const
    {
        return disable_checking || n != last_delete_align;
    }

    bool checkOutstandingArrayNewEq(int n) const
    {
        return disable_checking || n == outstanding_array_new;
    }

    bool checkOutstandingArrayNewNotEq(int n) const
    {
        return disable_checking || n != outstanding_array_new;
    }

    bool checkNewArrayCalledEq(int n) const
    {
        return disable_checking || n == new_array_called;
    }

    bool checkNewArrayCalledNotEq(int n) const
    {
        return disable_checking || n != new_array_called;
    }

    bool checkDeleteArrayCalledEq(int n) const
    {
        return disable_checking || n == delete_array_called;
    }

    bool checkDeleteArrayCalledNotEq(int n) const
    {
        return disable_checking || n != delete_array_called;
    }

    bool checkAlignedNewArrayCalledEq(int n) const
    {
        return disable_checking || n == aligned_new_array_called;
    }

    bool checkAlignedNewArrayCalledNotEq(int n) const
    {
        return disable_checking || n != aligned_new_array_called;
    }

    bool checkAlignedNewArrayCalledGreaterThan(int n) const
    {
        return disable_checking || aligned_new_array_called > n;
    }

    bool checkAlignedDeleteArrayCalledEq(int n) const
    {
        return disable_checking || n == aligned_delete_array_called;
    }

    bool checkAlignedDeleteArrayCalledNotEq(int n) const
    {
        return disable_checking || n != aligned_delete_array_called;
    }

    bool checkLastNewArraySizeEq(std::size_t n) const
    {
        return disable_checking || n == last_new_array_size;
    }

    bool checkLastNewArraySizeNotEq(std::size_t n) const
    {
        return disable_checking || n != last_new_array_size;
    }

    bool checkLastNewArrayAlignEq(std::size_t n) const
    {
        return disable_checking || n == last_new_array_align;
    }

    bool checkLastNewArrayAlignNotEq(std::size_t n) const
    {
        return disable_checking || n != last_new_array_align;
    }
};

#ifdef DISABLE_NEW_COUNT
  const bool MemCounter::disable_checking = true;
#else
  const bool MemCounter::disable_checking = false;
#endif

inline MemCounter* getGlobalMemCounter() {
  static MemCounter counter((MemCounter::MemCounterCtorArg_()));
  return &counter;
}

MemCounter &globalMemCounter = *getGlobalMemCounter();

#ifndef DISABLE_NEW_COUNT
void* operator new(std::size_t s) TEST_THROW_SPEC(std::bad_alloc)
{
    getGlobalMemCounter()->newCalled(s);
    void* ret = std::malloc(s);
    if (ret == nullptr)
        detail::throw_bad_alloc_helper();
    return ret;
}

void  operator delete(void* p) TEST_NOEXCEPT
{
    getGlobalMemCounter()->deleteCalled(p);
    std::free(p);
}

void* operator new[](std::size_t s) TEST_THROW_SPEC(std::bad_alloc)
{
    getGlobalMemCounter()->newArrayCalled(s);
    return operator new(s);
}

void operator delete[](void* p) TEST_NOEXCEPT
{
    getGlobalMemCounter()->deleteArrayCalled(p);
    operator delete(p);
}

#ifndef TEST_HAS_NO_ALIGNED_ALLOCATION
#if defined(_LIBCPP_MSVCRT_LIKE) || \
  (!defined(_LIBCPP_VERSION) && defined(_WIN32))
#define USE_ALIGNED_ALLOC
#endif

void* operator new(std::size_t s, std::align_val_t av) TEST_THROW_SPEC(std::bad_alloc) {
  const std::size_t a = static_cast<std::size_t>(av);
  getGlobalMemCounter()->alignedNewCalled(s, a);
  void *ret;
#ifdef USE_ALIGNED_ALLOC
  ret = _aligned_malloc(s, a);
#else
  posix_memalign(&ret, a, s);
#endif
  if (ret == nullptr)
    detail::throw_bad_alloc_helper();
  return ret;
}

void operator delete(void *p, std::align_val_t av) TEST_NOEXCEPT {
  const std::size_t a = static_cast<std::size_t>(av);
  getGlobalMemCounter()->alignedDeleteCalled(p, a);
  if (p) {
#ifdef USE_ALIGNED_ALLOC
    ::_aligned_free(p);
#else
    ::free(p);
#endif
  }
}

void* operator new[](std::size_t s, std::align_val_t av) TEST_THROW_SPEC(std::bad_alloc) {
  const std::size_t a = static_cast<std::size_t>(av);
  getGlobalMemCounter()->alignedNewArrayCalled(s, a);
  return operator new(s, av);
}

void operator delete[](void *p, std::align_val_t av) TEST_NOEXCEPT {
  const std::size_t a = static_cast<std::size_t>(av);
  getGlobalMemCounter()->alignedDeleteArrayCalled(p, a);
  return operator delete(p, av);
}

#endif // TEST_HAS_NO_ALIGNED_ALLOCATION

#endif // DISABLE_NEW_COUNT

struct DisableAllocationGuard {
    explicit DisableAllocationGuard(bool disable = true) : m_disabled(disable)
    {
        // Don't re-disable if already disabled.
        if (globalMemCounter.disable_allocations == true) m_disabled = false;
        if (m_disabled) globalMemCounter.disableAllocations();
    }

    void release() {
        if (m_disabled) globalMemCounter.enableAllocations();
        m_disabled = false;
    }

    ~DisableAllocationGuard() {
        release();
    }

private:
    bool m_disabled;

    DisableAllocationGuard(DisableAllocationGuard const&);
    DisableAllocationGuard& operator=(DisableAllocationGuard const&);
};

struct RequireAllocationGuard {
    explicit RequireAllocationGuard(std::size_t RequireAtLeast = 1)
            : m_req_alloc(RequireAtLeast),
              m_new_count_on_init(globalMemCounter.new_called),
              m_outstanding_new_on_init(globalMemCounter.outstanding_new),
              m_exactly(false)
    {
    }

    void requireAtLeast(std::size_t N) { m_req_alloc = N; m_exactly = false; }
    void requireExactly(std::size_t N) { m_req_alloc = N; m_exactly = true; }

    ~RequireAllocationGuard() {
        assert(globalMemCounter.checkOutstandingNewEq(static_cast<int>(m_outstanding_new_on_init)));
        std::size_t Expect = m_new_count_on_init + m_req_alloc;
        assert(globalMemCounter.checkNewCalledEq(static_cast<int>(Expect)) ||
               (!m_exactly && globalMemCounter.checkNewCalledGreaterThan(static_cast<int>(Expect))));
    }

private:
    std::size_t m_req_alloc;
    const std::size_t m_new_count_on_init;
    const std::size_t m_outstanding_new_on_init;
    bool m_exactly;
    RequireAllocationGuard(RequireAllocationGuard const&);
    RequireAllocationGuard& operator=(RequireAllocationGuard const&);
};

#endif /* COUNT_NEW_HPP */
