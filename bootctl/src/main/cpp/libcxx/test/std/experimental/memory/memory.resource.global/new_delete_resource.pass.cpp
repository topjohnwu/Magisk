//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// REQUIRES: c++experimental
// UNSUPPORTED: c++98, c++03

// <experimental/memory_resource>

// memory_resource * new_delete_resource()

#include <experimental/memory_resource>
#include <type_traits>
#include <cassert>

#include "count_new.hpp"

namespace ex = std::experimental::pmr;

struct assert_on_compare : public ex::memory_resource
{
protected:
    virtual void * do_allocate(size_t, size_t)
    { assert(false); }

    virtual void do_deallocate(void *, size_t, size_t)
    { assert(false); }

    virtual bool do_is_equal(ex::memory_resource const &) const noexcept
    { assert(false); }
};

void test_return()
{
    {
        static_assert(std::is_same<
            decltype(ex::new_delete_resource()), ex::memory_resource*
          >::value, "");
    }
    // assert not null
    {
        assert(ex::new_delete_resource());
    }
    // assert same return value
    {
        assert(ex::new_delete_resource() == ex::new_delete_resource());
    }
}

void test_equality()
{
    // Same object
    {
        ex::memory_resource & r1 = *ex::new_delete_resource();
        ex::memory_resource & r2 = *ex::new_delete_resource();
        // check both calls returned the same object
        assert(&r1 == &r2);
        // check for proper equality semantics
        assert(r1 == r2);
        assert(r2 == r1);
        assert(!(r1 != r2));
        assert(!(r2 != r1));
    }
    // Different types
    {
        ex::memory_resource & r1 = *ex::new_delete_resource();
        assert_on_compare c;
        ex::memory_resource & r2 = c;
        assert(r1 != r2);
        assert(!(r1 == r2));
    }
}

void test_allocate_deallocate()
{
    ex::memory_resource & r1 = *ex::new_delete_resource();

    globalMemCounter.reset();

    void *ret = r1.allocate(50);
    assert(ret);
    assert(globalMemCounter.checkOutstandingNewEq(1));
    assert(globalMemCounter.checkLastNewSizeEq(50));

    r1.deallocate(ret, 1);
    assert(globalMemCounter.checkOutstandingNewEq(0));
    assert(globalMemCounter.checkDeleteCalledEq(1));

}

int main()
{
    static_assert(noexcept(ex::new_delete_resource()), "Must be noexcept");
    test_return();
    test_equality();
    test_allocate_deallocate();
}
