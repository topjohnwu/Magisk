//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: libcpp-no-exceptions

// <algorithm>

// template <class _Compare> struct __debug_less

// __debug_less checks that a comparator actually provides a strict-weak ordering.

struct DebugException {};

#define _LIBCPP_DEBUG 0
#define _LIBCPP_ASSERT(x, m) ((x) ? (void)0 : throw ::DebugException())

#include <algorithm>
#include <cassert>

template <int ID>
struct MyType {
    int value;
    explicit MyType(int xvalue = 0) : value(xvalue) {}
};

template <int ID1, int ID2>
bool operator<(MyType<ID1> const& LHS, MyType<ID2> const& RHS) {
    return LHS.value < RHS.value;
}

struct CompareBase {
    static int called;
    static void reset() {
        called = 0;
    }
};

int CompareBase::called = 0;

template <class ValueType>
struct GoodComparator : public CompareBase {
    bool operator()(ValueType const& lhs, ValueType const& rhs) const {
        ++CompareBase::called;
        return lhs < rhs;
    }
};

template <class ValueType>
struct BadComparator : public CompareBase {
    bool operator()(ValueType const&, ValueType const&) const {
        ++CompareBase::called;
        return true;
    }
};

template <class T1, class T2>
struct TwoWayHomoComparator : public CompareBase {
    bool operator()(T1 const& lhs, T2 const& rhs) const {
        ++CompareBase::called;
        return lhs < rhs;
    }

    bool operator()(T2 const& lhs, T1 const& rhs) const {
        ++CompareBase::called;
        return lhs < rhs;
    }
};

template <class T1, class T2>
struct OneWayHomoComparator : public CompareBase {
    bool operator()(T1 const& lhs, T2 const& rhs) const {
        ++CompareBase::called;
        return lhs < rhs;
    }
};

using std::__debug_less;

typedef MyType<0> MT0;
typedef MyType<1> MT1;

void test_passing() {
    int& called = CompareBase::called;
    called = 0;
    MT0 one(1);
    MT0 two(2);
    MT1 three(3);
    MT1 four(4);

    {
        typedef GoodComparator<MT0> C;
        typedef __debug_less<C> D;

        C c;
        D d(c);

        assert(d(one, two) == true);
        assert(called == 2);
        called = 0;

        assert(d(one, one) == false);
        assert(called == 1);
        called = 0;

        assert(d(two, one) == false);
        assert(called == 1);
        called = 0;
    }
    {
        typedef TwoWayHomoComparator<MT0, MT1> C;
        typedef __debug_less<C> D;
        C c;
        D d(c);

        assert(d(one, three) == true);
        assert(called == 2);
        called = 0;

        assert(d(three, one) == false);
        assert(called == 1);
        called = 0;
    }
    {
        typedef OneWayHomoComparator<MT0, MT1> C;
        typedef __debug_less<C> D;
        C c;
        D d(c);

        assert(d(one, three) == true);
        assert(called == 1);
        called = 0;
    }
}

void test_failing() {
    int& called = CompareBase::called;
    called = 0;
    MT0 one(1);
    MT0 two(2);

    {
        typedef BadComparator<MT0> C;
        typedef __debug_less<C> D;
        C c;
        D d(c);

        try {
            d(one, two);
            assert(false);
        } catch (DebugException const&) {
        }

        assert(called == 2);
        called = 0;
    }
}

template <int>
struct Tag {
  explicit Tag(int v) : value(v) {}
  int value;
};

template <class = void>
struct FooImp {
  explicit FooImp(int x) : x_(x) {}
  int x_;
};

template <class T>
inline bool operator<(FooImp<T> const& x, Tag<0> y) {
    return x.x_ < y.value;
}

template <class T>
inline bool operator<(Tag<0>, FooImp<T> const&) {
    static_assert(sizeof(FooImp<T>) != sizeof(FooImp<T>), "should not be instantiated");
}

template <class T>
inline bool operator<(Tag<1> x, FooImp<T> const& y) {
    return x.value < y.x_;
}

template <class T>
inline bool operator<(FooImp<T> const&, Tag<1>) {
    static_assert(sizeof(FooImp<T>) != sizeof(FooImp<T>), "should not be instantiated");
}

typedef FooImp<> Foo;

// Test that we don't attempt to call the comparator with the arguments reversed
// for upper_bound and lower_bound since the comparator or type is not required
// to support it, nor does it require the range to have a strict weak ordering.
// See llvm.org/PR39458
void test_upper_and_lower_bound() {
    Foo table[] = {Foo(1), Foo(2), Foo(3), Foo(4), Foo(5)};
    {
        Foo* iter = std::lower_bound(std::begin(table), std::end(table), Tag<0>(3));
        assert(iter == (table + 2));
    }
    {
        Foo* iter = std::upper_bound(std::begin(table), std::end(table), Tag<1>(3));
        assert(iter == (table + 3));
    }
}

int main() {
    test_passing();
    test_failing();
    test_upper_and_lower_bound();
}
