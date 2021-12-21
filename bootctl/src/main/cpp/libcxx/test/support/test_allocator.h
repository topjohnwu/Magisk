//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef TEST_ALLOCATOR_H
#define TEST_ALLOCATOR_H

#include <type_traits>
#include <new>
#include <memory>
#include <utility>
#include <cstddef>
#include <cstdlib>
#include <climits>
#include <cassert>

#include "test_macros.h"

template <class Alloc>
inline typename std::allocator_traits<Alloc>::size_type
alloc_max_size(Alloc const &a) {
  typedef std::allocator_traits<Alloc> AT;
  return AT::max_size(a);
}

class test_alloc_base
{
protected:
    static int time_to_throw;
public:
    static int throw_after;
    static int count;
    static int alloc_count;
    static int copied;
    static int moved;
    static int converted;

    const static int destructed_value = -1;
    const static int default_value = 0;
    const static int moved_value = INT_MAX;

    static void clear() {
      assert(count == 0 && "clearing leaking allocator data?");
      count = 0;
      time_to_throw = 0;
      alloc_count = 0;
      throw_after = INT_MAX;
      clear_ctor_counters();
    }

    static void clear_ctor_counters() {
      copied = 0;
      moved = 0;
      converted = 0;
    }
};

int test_alloc_base::count = 0;
int test_alloc_base::time_to_throw = 0;
int test_alloc_base::alloc_count = 0;
int test_alloc_base::throw_after = INT_MAX;
int test_alloc_base::copied = 0;
int test_alloc_base::moved = 0;
int test_alloc_base::converted = 0;

template <class T>
class test_allocator
    : public test_alloc_base
{
    int data_; // participates in equality
    int id_; // unique identifier, doesn't participate in equality
    template <class U> friend class test_allocator;
public:

    typedef unsigned                                                   size_type;
    typedef int                                                        difference_type;
    typedef T                                                          value_type;
    typedef value_type*                                                pointer;
    typedef const value_type*                                          const_pointer;
    typedef typename std::add_lvalue_reference<value_type>::type       reference;
    typedef typename std::add_lvalue_reference<const value_type>::type const_reference;

    template <class U> struct rebind {typedef test_allocator<U> other;};

    test_allocator() TEST_NOEXCEPT : data_(0), id_(0) {++count;}
    explicit test_allocator(int i, int id = 0) TEST_NOEXCEPT : data_(i), id_(id)
      {++count;}
    test_allocator(const test_allocator& a) TEST_NOEXCEPT : data_(a.data_),
                                                            id_(a.id_) {
      ++count;
      ++copied;
      assert(a.data_ != destructed_value && a.id_ != destructed_value &&
             "copying from destroyed allocator");
    }
#if TEST_STD_VER >= 11
    test_allocator(test_allocator&& a) TEST_NOEXCEPT : data_(a.data_),
                                                       id_(a.id_) {
      ++count;
      ++moved;
      assert(a.data_ != destructed_value && a.id_ != destructed_value &&
             "moving from destroyed allocator");
      a.data_ = moved_value;
      a.id_ = moved_value;
    }
#endif
    template <class U>
    test_allocator(const test_allocator<U>& a) TEST_NOEXCEPT : data_(a.data_),
                                                               id_(a.id_) {
      ++count;
      ++converted;
    }
    ~test_allocator() TEST_NOEXCEPT {
      assert(data_ >= 0); assert(id_ >= 0);
      --count;
      data_ = destructed_value;
      id_ = destructed_value;
    }
    pointer address(reference x) const {return &x;}
    const_pointer address(const_reference x) const {return &x;}
    pointer allocate(size_type n, const void* = 0)
        {
            assert(data_ >= 0);
            if (time_to_throw >= throw_after) {
#ifndef TEST_HAS_NO_EXCEPTIONS
                throw std::bad_alloc();
#else
                std::terminate();
#endif
            }
            ++time_to_throw;
            ++alloc_count;
            return (pointer)::operator new(n * sizeof(T));
        }
    void deallocate(pointer p, size_type)
        {assert(data_ >= 0); --alloc_count; ::operator delete((void*)p);}
    size_type max_size() const TEST_NOEXCEPT
        {return UINT_MAX / sizeof(T);}
#if TEST_STD_VER < 11
    void construct(pointer p, const T& val)
        {::new(static_cast<void*>(p)) T(val);}
#else
    template <class U> void construct(pointer p, U&& val)
        {::new(static_cast<void*>(p)) T(std::forward<U>(val));}
#endif
    void destroy(pointer p)
        {p->~T();}
    friend bool operator==(const test_allocator& x, const test_allocator& y)
        {return x.data_ == y.data_;}
    friend bool operator!=(const test_allocator& x, const test_allocator& y)
        {return !(x == y);}

    int get_data() const { return data_; }
    int get_id() const { return id_; }
};

template <class T>
class non_default_test_allocator
    : public test_alloc_base
{
    int data_;

    template <class U> friend class non_default_test_allocator;
public:

    typedef unsigned                                                   size_type;
    typedef int                                                        difference_type;
    typedef T                                                          value_type;
    typedef value_type*                                                pointer;
    typedef const value_type*                                          const_pointer;
    typedef typename std::add_lvalue_reference<value_type>::type       reference;
    typedef typename std::add_lvalue_reference<const value_type>::type const_reference;

    template <class U> struct rebind {typedef non_default_test_allocator<U> other;};

//    non_default_test_allocator() TEST_NOEXCEPT : data_(0) {++count;}
    explicit non_default_test_allocator(int i) TEST_NOEXCEPT : data_(i) {++count;}
    non_default_test_allocator(const non_default_test_allocator& a) TEST_NOEXCEPT
        : data_(a.data_) {++count;}
    template <class U> non_default_test_allocator(const non_default_test_allocator<U>& a) TEST_NOEXCEPT
        : data_(a.data_) {++count;}
    ~non_default_test_allocator() TEST_NOEXCEPT {assert(data_ >= 0); --count; data_ = -1;}
    pointer address(reference x) const {return &x;}
    const_pointer address(const_reference x) const {return &x;}
    pointer allocate(size_type n, const void* = 0)
        {
            assert(data_ >= 0);
            if (time_to_throw >= throw_after) {
#ifndef TEST_HAS_NO_EXCEPTIONS
                throw std::bad_alloc();
#else
                std::terminate();
#endif
            }
            ++time_to_throw;
            ++alloc_count;
            return (pointer)::operator new (n * sizeof(T));
        }
    void deallocate(pointer p, size_type)
        {assert(data_ >= 0); --alloc_count; ::operator delete((void*)p); }
    size_type max_size() const TEST_NOEXCEPT
        {return UINT_MAX / sizeof(T);}
#if TEST_STD_VER < 11
    void construct(pointer p, const T& val)
        {::new(static_cast<void*>(p)) T(val);}
#else
    template <class U> void construct(pointer p, U&& val)
        {::new(static_cast<void*>(p)) T(std::forward<U>(val));}
#endif
    void destroy(pointer p) {p->~T();}

    friend bool operator==(const non_default_test_allocator& x, const non_default_test_allocator& y)
        {return x.data_ == y.data_;}
    friend bool operator!=(const non_default_test_allocator& x, const non_default_test_allocator& y)
        {return !(x == y);}
};

template <>
class test_allocator<void>
    : public test_alloc_base
{
    int data_;
    int id_;

    template <class U> friend class test_allocator;
public:

    typedef unsigned                                                   size_type;
    typedef int                                                        difference_type;
    typedef void                                                       value_type;
    typedef value_type*                                                pointer;
    typedef const value_type*                                          const_pointer;

    template <class U> struct rebind {typedef test_allocator<U> other;};

    test_allocator() TEST_NOEXCEPT : data_(0), id_(0) {}
    explicit test_allocator(int i, int id = 0) TEST_NOEXCEPT : data_(i), id_(id) {}
    test_allocator(const test_allocator& a) TEST_NOEXCEPT
        : data_(a.data_), id_(a.id_) {}
    template <class U> test_allocator(const test_allocator<U>& a) TEST_NOEXCEPT
        : data_(a.data_), id_(a.id_) {}
    ~test_allocator() TEST_NOEXCEPT {data_ = -1; id_ = -1; }

    int get_id() const { return id_; }
    int get_data() const { return data_; }

    friend bool operator==(const test_allocator& x, const test_allocator& y)
        {return x.data_ == y.data_;}
    friend bool operator!=(const test_allocator& x, const test_allocator& y)
        {return !(x == y);}
};

template <class T>
class other_allocator
{
    int data_;

    template <class U> friend class other_allocator;

public:
    typedef T value_type;

    other_allocator() : data_(-1) {}
    explicit other_allocator(int i) : data_(i) {}
    template <class U> other_allocator(const other_allocator<U>& a)
        : data_(a.data_) {}
    T* allocate(std::size_t n)
        {return (T*)::operator new(n * sizeof(T));}
    void deallocate(T* p, std::size_t)
        {::operator delete((void*)p);}

    other_allocator select_on_container_copy_construction() const
        {return other_allocator(-2);}

    friend bool operator==(const other_allocator& x, const other_allocator& y)
        {return x.data_ == y.data_;}
    friend bool operator!=(const other_allocator& x, const other_allocator& y)
        {return !(x == y);}

    typedef std::true_type propagate_on_container_copy_assignment;
    typedef std::true_type propagate_on_container_move_assignment;
    typedef std::true_type propagate_on_container_swap;

#if TEST_STD_VER < 11
    std::size_t max_size() const
        {return UINT_MAX / sizeof(T);}
#endif

};

#if TEST_STD_VER >= 11

struct Ctor_Tag {};

template <typename T> class TaggingAllocator;

struct Tag_X {
  // All constructors must be passed the Tag type.

  // DefaultInsertable into vector<X, TaggingAllocator<X>>,
  Tag_X(Ctor_Tag) {}
  // CopyInsertable into vector<X, TaggingAllocator<X>>,
  Tag_X(Ctor_Tag, const Tag_X&) {}
  // MoveInsertable into vector<X, TaggingAllocator<X>>, and
  Tag_X(Ctor_Tag, Tag_X&&) {}

  // EmplaceConstructible into vector<X, TaggingAllocator<X>> from args.
  template<typename... Args>
  Tag_X(Ctor_Tag, Args&&...) { }

  // not DefaultConstructible, CopyConstructible or MoveConstructible.
  Tag_X() = delete;
  Tag_X(const Tag_X&) = delete;
  Tag_X(Tag_X&&) = delete;

  // CopyAssignable.
  Tag_X& operator=(const Tag_X&) { return *this; }

  // MoveAssignable.
  Tag_X& operator=(Tag_X&&) { return *this; }

private:
  // Not Destructible.
  ~Tag_X() { }

  // Erasable from vector<X, TaggingAllocator<X>>.
  friend class TaggingAllocator<Tag_X>;
};


template<typename T>
class TaggingAllocator {
public:
    using value_type = T;
    TaggingAllocator() = default;

    template<typename U>
      TaggingAllocator(const TaggingAllocator<U>&) { }

    T* allocate(std::size_t n) { return std::allocator<T>{}.allocate(n); }

    void deallocate(T* p, std::size_t n) { std::allocator<T>{}.deallocate(p, n); }

    template<typename... Args>
    void construct(Tag_X* p, Args&&... args)
    { ::new((void*)p) Tag_X(Ctor_Tag{}, std::forward<Args>(args)...); }

    template<typename U, typename... Args>
    void construct(U* p, Args&&... args)
    { ::new((void*)p) U(std::forward<Args>(args)...); }

    template<typename U, typename... Args>
    void destroy(U* p)
    { p->~U(); }
};

template<typename T, typename U>
bool
operator==(const TaggingAllocator<T>&, const TaggingAllocator<U>&)
{ return true; }

template<typename T, typename U>
bool
operator!=(const TaggingAllocator<T>&, const TaggingAllocator<U>&)
{ return false; }
#endif

template <std::size_t MaxAllocs>
struct limited_alloc_handle {
  std::size_t outstanding_;
  void* last_alloc_;

  limited_alloc_handle() : outstanding_(0), last_alloc_(nullptr) {}

  template <class T>
  T *allocate(std::size_t N) {
    if (N + outstanding_ > MaxAllocs)
      TEST_THROW(std::bad_alloc());
    last_alloc_ = ::operator new(N*sizeof(T));
    outstanding_ += N;
    return static_cast<T*>(last_alloc_);
  }

  void deallocate(void* ptr, std::size_t N) {
    if (ptr == last_alloc_) {
      last_alloc_ = nullptr;
      assert(outstanding_ >= N);
      outstanding_ -= N;
    }
    ::operator delete(ptr);
  }
};

template <class T, std::size_t N>
class limited_allocator
{
    template <class U, std::size_t UN> friend class limited_allocator;
    typedef limited_alloc_handle<N> BuffT;
    std::shared_ptr<BuffT> handle_;
public:
    typedef T                 value_type;
    typedef value_type*       pointer;
    typedef const value_type* const_pointer;
    typedef value_type&       reference;
    typedef const value_type& const_reference;
    typedef std::size_t       size_type;
    typedef std::ptrdiff_t    difference_type;

    template <class U> struct rebind { typedef limited_allocator<U, N> other; };

    limited_allocator() : handle_(new BuffT) {}

    limited_allocator(limited_allocator const& other) : handle_(other.handle_) {}

    template <class U>
    explicit limited_allocator(limited_allocator<U, N> const& other)
        : handle_(other.handle_) {}

private:
    limited_allocator& operator=(const limited_allocator&);// = delete;

public:
    pointer allocate(size_type n) { return handle_->template allocate<T>(n); }
    void deallocate(pointer p, size_type n) { handle_->deallocate(p, n); }
    size_type max_size() const {return N;}

    BuffT* getHandle() const { return handle_.get(); }
};

template <class T, class U, std::size_t N>
inline bool operator==(limited_allocator<T, N> const& LHS,
                       limited_allocator<U, N> const& RHS) {
  return LHS.getHandle() == RHS.getHandle();
}

template <class T, class U, std::size_t N>
inline bool operator!=(limited_allocator<T, N> const& LHS,
                       limited_allocator<U, N> const& RHS) {
  return !(LHS == RHS);
}


#endif  // TEST_ALLOCATOR_H
