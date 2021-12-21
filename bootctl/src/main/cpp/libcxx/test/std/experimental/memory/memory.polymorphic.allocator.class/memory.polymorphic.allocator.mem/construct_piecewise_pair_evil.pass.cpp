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

// <memory_resource>

// template <class T> class polymorphic_allocator

// template <class U1, class U2, class ...Args1, class ...Args2>
// void polymorphic_allocator<T>::construct(pair<U1, U2>*, piecewise_construct_t
//                                          tuple<Args1...>, tuple<Args2...>)

#include <experimental/memory_resource>
#include <tuple>
#include <type_traits>
#include <utility>
#include <cassert>
#include <cstdlib>

#include "test_macros.h"

namespace ex = std::experimental::pmr;

template <class T>
struct EvilAlloc {
    explicit EvilAlloc() : inner_(ex::null_memory_resource()) {}

    EvilAlloc(ex::polymorphic_allocator<T> & a) : inner_(a) {}
    EvilAlloc(ex::polymorphic_allocator<T> && a) : inner_(a) {}
    EvilAlloc(ex::polymorphic_allocator<T> const & a) = delete;
    EvilAlloc(ex::polymorphic_allocator<T> const && a) = delete;

    using value_type = T;
    template <class U> EvilAlloc(EvilAlloc<U> const & rhs) : inner_(rhs.inner_) {}

    ex::polymorphic_allocator<T> inner_;
};

struct WidgetV0 {
    WidgetV0(int v) : value_(v) {}

    bool holds(int v, const ex::polymorphic_allocator<char>&) const {
        return value_ == v;
    }
private:
    int value_;
};

struct WidgetV1 {
    using allocator_type = EvilAlloc<char>;

    WidgetV1(int v) : value_(v), alloc_() {}
    WidgetV1(std::allocator_arg_t, EvilAlloc<char> a, int v) : value_(v), alloc_(a) {}

    bool holds(int v, const ex::polymorphic_allocator<char>& a) const {
        return value_ == v && alloc_.inner_ == a;
    }
private:
    int value_;
    EvilAlloc<char> alloc_;
};

struct WidgetV2 {
    using allocator_type = EvilAlloc<char>;

    WidgetV2(int v) : value_(v), alloc_() {}
    WidgetV2(int v, EvilAlloc<char> a) : value_(v), alloc_(a) {}

    bool holds(int v, ex::polymorphic_allocator<char> a) const {
        return value_ == v && alloc_.inner_ == a;
    }
private:
    int value_;
    EvilAlloc<char> alloc_;
};

struct WidgetV3 {
    using allocator_type = EvilAlloc<char>;

    WidgetV3(int v) : value_(v), alloc_() {}
    WidgetV3(std::allocator_arg_t, EvilAlloc<char> a, int v) : value_(v), alloc_(a) {}
    WidgetV3(int v, EvilAlloc<char> a) : value_(v), alloc_(a) {}

    bool holds(int v, ex::polymorphic_allocator<char> a) const {
        return value_ == v && alloc_.inner_ == a;
    }
private:
    int value_;
    EvilAlloc<char> alloc_;
};

static_assert(std::uses_allocator<WidgetV1, EvilAlloc<char>>::value, "");
static_assert(std::uses_allocator<WidgetV2, EvilAlloc<char>>::value, "");
static_assert(std::uses_allocator<WidgetV3, EvilAlloc<char>>::value, "");
static_assert(std::uses_allocator<WidgetV1, ex::polymorphic_allocator<char>>::value, "");
static_assert(std::uses_allocator<WidgetV2, ex::polymorphic_allocator<char>>::value, "");
static_assert(std::uses_allocator<WidgetV3, ex::polymorphic_allocator<char>>::value, "");

template<class W1, class W2>
void test_evil()
{
    using PMA = ex::polymorphic_allocator<char>;
    PMA pma(ex::new_delete_resource());
    {
        using Pair = std::pair<W1, W2>;
        void *where = std::malloc(sizeof (Pair));
        Pair *p = (Pair *)where;
        pma.construct(p, std::piecewise_construct, std::make_tuple(42), std::make_tuple(42));
        assert(p->first.holds(42, pma));
        assert(p->second.holds(42, pma));
        pma.destroy(p);
        std::free(where);
    }
}

int main()
{
    test_evil<WidgetV0, WidgetV0>();
    test_evil<WidgetV0, WidgetV1>();
    test_evil<WidgetV0, WidgetV2>();
    test_evil<WidgetV0, WidgetV3>();
    test_evil<WidgetV1, WidgetV0>();
    test_evil<WidgetV1, WidgetV1>();
    test_evil<WidgetV1, WidgetV2>();
    test_evil<WidgetV1, WidgetV3>();
    test_evil<WidgetV2, WidgetV0>();
    test_evil<WidgetV2, WidgetV1>();
    test_evil<WidgetV2, WidgetV2>();
    test_evil<WidgetV2, WidgetV3>();
    test_evil<WidgetV3, WidgetV0>();
    test_evil<WidgetV3, WidgetV1>();
    test_evil<WidgetV3, WidgetV2>();
    test_evil<WidgetV3, WidgetV3>();
}
