//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <memory>

// template <class T> struct owner_less;
//
// template <class T>
// struct owner_less<shared_ptr<T> >
//     : binary_function<shared_ptr<T>, shared_ptr<T>, bool>
// {
//     typedef bool result_type;
//     bool operator()(shared_ptr<T> const&, shared_ptr<T> const&) const noexcept;
//     bool operator()(shared_ptr<T> const&, weak_ptr<T> const&) const noexcept;
//     bool operator()(weak_ptr<T> const&, shared_ptr<T> const&) const noexcept;
// };
//
// template <class T>
// struct owner_less<weak_ptr<T> >
//     : binary_function<weak_ptr<T>, weak_ptr<T>, bool>
// {
//     typedef bool result_type;
//     bool operator()(weak_ptr<T> const&, weak_ptr<T> const&) const noexcept;
//     bool operator()(shared_ptr<T> const&, weak_ptr<T> const&) const noexcept;
//     bool operator()(weak_ptr<T> const&, shared_ptr<T> const&) const noexcept;
// };
//
// Added in C++17
// template<> struct owner_less<void>
// {
//     template<class T, class U>
//         bool operator()(shared_ptr<T> const&, shared_ptr<U> const&) const noexcept;
//     template<class T, class U>
//         bool operator()(shared_ptr<T> const&, weak_ptr<U> const&) const noexcept;
//     template<class T, class U>
//         bool operator()(weak_ptr<T> const&, shared_ptr<U> const&) const noexcept;
//     template<class T, class U>
//         bool operator()(weak_ptr<T> const&, weak_ptr<U> const&) const noexcept;
//
//     typedef unspecified is_transparent;
// };

#include <memory>
#include <cassert>
#include <set>
#include "test_macros.h"

struct X {};

int main()
{
    const std::shared_ptr<int> p1(new int);
    const std::shared_ptr<int> p2 = p1;
    const std::shared_ptr<int> p3(new int);
    const std::weak_ptr<int> w1(p1);
    const std::weak_ptr<int> w2(p2);
    const std::weak_ptr<int> w3(p3);

    {
    typedef std::owner_less<std::shared_ptr<int> > CS;
    CS cs;

    static_assert((std::is_same<std::shared_ptr<int>, CS::first_argument_type>::value), "" );
    static_assert((std::is_same<std::shared_ptr<int>, CS::second_argument_type>::value), "" );
    static_assert((std::is_same<bool, CS::result_type>::value), "" );

    assert(!cs(p1, p2));
    assert(!cs(p2, p1));
    assert(cs(p1 ,p3) || cs(p3, p1));
    assert(cs(p3, p1) == cs(p3, p2));
    ASSERT_NOEXCEPT(cs(p1, p1));

    assert(!cs(p1, w2));
    assert(!cs(p2, w1));
    assert(cs(p1, w3) || cs(p3, w1));
    assert(cs(p3, w1) == cs(p3, w2));
    ASSERT_NOEXCEPT(cs(p1, w1));
    ASSERT_NOEXCEPT(cs(w1, p1));
    }
    {
    typedef std::owner_less<std::weak_ptr<int> > CS;
    CS cs;

    static_assert((std::is_same<std::weak_ptr<int>, CS::first_argument_type>::value), "" );
    static_assert((std::is_same<std::weak_ptr<int>, CS::second_argument_type>::value), "" );
    static_assert((std::is_same<bool, CS::result_type>::value), "" );

    assert(!cs(w1, w2));
    assert(!cs(w2, w1));
    assert(cs(w1, w3) || cs(w3, w1));
    assert(cs(w3, w1) == cs(w3, w2));
    ASSERT_NOEXCEPT(cs(w1, w1));

    assert(!cs(w1, p2));
    assert(!cs(w2, p1));
    assert(cs(w1, p3) || cs(w3, p1));
    assert(cs(w3, p1) == cs(w3, p2));
    ASSERT_NOEXCEPT(cs(w1, p1));
    ASSERT_NOEXCEPT(cs(p1, w1));
    }
#if TEST_STD_VER > 14
    {
    std::shared_ptr<int> sp1;
    std::shared_ptr<void> sp2;
    std::shared_ptr<long> sp3;
    std::weak_ptr<int> wp1;

    std::owner_less<> cmp;
    assert(!cmp(sp1, sp2));
    assert(!cmp(sp1, wp1));
    assert(!cmp(sp1, sp3));
    assert(!cmp(wp1, sp1));
    assert(!cmp(wp1, wp1));
    ASSERT_NOEXCEPT(cmp(sp1, sp1));
    ASSERT_NOEXCEPT(cmp(sp1, wp1));
    ASSERT_NOEXCEPT(cmp(wp1, sp1));
    ASSERT_NOEXCEPT(cmp(wp1, wp1));
    }
    {
    // test heterogeneous lookups
    std::set<std::shared_ptr<X>, std::owner_less<>> s;
    std::shared_ptr<void> vp;
    assert(s.find(vp) == s.end());
    }
#endif
}
