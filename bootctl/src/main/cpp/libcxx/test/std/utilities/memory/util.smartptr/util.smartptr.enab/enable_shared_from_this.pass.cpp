//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// template<class T>
// class enable_shared_from_this
// {
// protected:
//     enable_shared_from_this();
//     enable_shared_from_this(enable_shared_from_this const&);
//     enable_shared_from_this& operator=(enable_shared_from_this const&);
//     ~enable_shared_from_this();
// public:
//     shared_ptr<T> shared_from_this();
//     shared_ptr<T const> shared_from_this() const;
//     weak_ptr<T> weak_from_this() noexcept;                         // C++17
//     weak_ptr<T const> weak_from_this() const noexecpt;             // C++17
// };

#include <memory>
#include <cassert>

#include "test_macros.h"
#include "count_new.hpp"

struct T
    : public std::enable_shared_from_this<T>
{
};

struct Y : T {};

struct Z : Y {};

void nullDeleter(void*) {}

struct Foo : virtual public std::enable_shared_from_this<Foo>
{
    virtual ~Foo() {}
};

struct Bar : public Foo {
    Bar(int) {}
};


struct PrivateBase : private std::enable_shared_from_this<PrivateBase> {
};


int main()
{
    {  // https://bugs.llvm.org/show_bug.cgi?id=18843
    std::shared_ptr<T const> t1(new T);
    std::shared_ptr<T const> t2(std::make_shared<T>());
    }
    { // https://bugs.llvm.org/show_bug.cgi?id=27115
    int x = 42;
    std::shared_ptr<Bar> t1(new Bar(42));
    assert(t1->shared_from_this() == t1);
    std::shared_ptr<Bar> t2(std::make_shared<Bar>(x));
    assert(t2->shared_from_this() == t2);
    }
    {
    std::shared_ptr<Y> p(new Z);
    std::shared_ptr<T> q = p->shared_from_this();
    assert(p == q);
    assert(!p.owner_before(q) && !q.owner_before(p)); // p and q share ownership
    }
    {
    std::shared_ptr<Y> p = std::make_shared<Z>();
    std::shared_ptr<T> q = p->shared_from_this();
    assert(p == q);
    assert(!p.owner_before(q) && !q.owner_before(p)); // p and q share ownership
    }
    {
      typedef std::shared_ptr<PrivateBase> APtr;
      APtr a1 = std::make_shared<PrivateBase>();
      assert(a1.use_count() == 1);
    }
    // Test LWG issue 2529. Only reset '__weak_ptr_' when it's already expired.
    // https://cplusplus.github.io/LWG/lwg-defects.html#2529
    // Test two different ways:
    // * Using 'weak_from_this().expired()' in C++17.
    // * Using 'shared_from_this()' in all dialects.
    {
        assert(globalMemCounter.checkOutstandingNewEq(0));
        T* ptr = new T;
        std::shared_ptr<T> s(ptr);
        {
            // Don't re-initialize the "enable_shared_from_this" base
            // because it already references a non-expired shared_ptr.
            std::shared_ptr<T> s2(ptr, &nullDeleter);
        }
#if TEST_STD_VER > 14
        // The enable_shared_from_this base should still be referencing
        // the original shared_ptr.
        assert(!ptr->weak_from_this().expired());
#endif
#ifndef TEST_HAS_NO_EXCEPTIONS
        {
            try {
                std::shared_ptr<T> new_s = ptr->shared_from_this();
                assert(new_s == s);
            } catch (std::bad_weak_ptr const&) {
                assert(false);
            } catch (...) {
                assert(false);
            }
        }
#endif
        s.reset();
        assert(globalMemCounter.checkOutstandingNewEq(0));
    }
    // Test LWG issue 2529 again. This time check that an expired pointer
    // is replaced.
    {
        assert(globalMemCounter.checkOutstandingNewEq(0));
        T* ptr = new T;
        std::weak_ptr<T> weak;
        {
            std::shared_ptr<T> s(ptr, &nullDeleter);
            assert(ptr->shared_from_this() == s);
            weak = s;
            assert(!weak.expired());
        }
        assert(weak.expired());
        weak.reset();

#ifndef TEST_HAS_NO_EXCEPTIONS
        try {
            TEST_IGNORE_NODISCARD ptr->shared_from_this();
            assert(false);
        } catch (std::bad_weak_ptr const&) {
        } catch (...) { assert(false); }
#endif
        {
            std::shared_ptr<T> s2(ptr, &nullDeleter);
            assert(ptr->shared_from_this() == s2);
        }
        delete ptr;
        assert(globalMemCounter.checkOutstandingNewEq(0));
    }
    // Test weak_from_this_methods
#if TEST_STD_VER > 14
    {
        T* ptr = new T;
        const T* cptr = ptr;

        static_assert(noexcept(ptr->weak_from_this()), "Operation must be noexcept");
        static_assert(noexcept(cptr->weak_from_this()), "Operation must be noexcept");

        std::weak_ptr<T> my_weak = ptr->weak_from_this();
        assert(my_weak.expired());

        std::weak_ptr<T const> my_const_weak = cptr->weak_from_this();
        assert(my_const_weak.expired());

        // Enable shared_from_this with ptr.
        std::shared_ptr<T> sptr(ptr);
        my_weak = ptr->weak_from_this();
        assert(!my_weak.expired());
        assert(my_weak.lock().get() == ptr);
    }
#endif
}
