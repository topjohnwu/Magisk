//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// basic_string(const basic_string& str, const Allocator& alloc);

#include <string>
#include <cassert>

#include "test_macros.h"
#include "test_allocator.h"
#include "min_allocator.h"

#ifndef TEST_HAS_NO_EXCEPTIONS
template <class T>
struct alloc_imp {
    bool active;

    alloc_imp() : active(true) {}

    T* allocate(std::size_t n)
    {
        if (active)
            return static_cast<T*>(std::malloc(n * sizeof(T)));
        else
            throw std::bad_alloc();
    }

    void deallocate(T* p, std::size_t) { std::free(p); }
    void activate  ()                  { active = true; }
    void deactivate()                  { active = false; }
};

template <class T>
struct poca_alloc {
    typedef T value_type;
    typedef std::true_type propagate_on_container_copy_assignment;

    alloc_imp<T> *imp;

    poca_alloc(alloc_imp<T> *imp_) : imp (imp_) {}

    template <class U>
    poca_alloc(const poca_alloc<U>& other) : imp(other.imp) {}

    T*   allocate  (std::size_t n)       { return imp->allocate(n);}
    void deallocate(T* p, std::size_t n) { imp->deallocate(p, n); }
};

template <typename T, typename U>
bool operator==(const poca_alloc<T>& lhs, const poca_alloc<U>& rhs)
{
    return lhs.imp == rhs.imp;
}

template <typename T, typename U>
bool operator!=(const poca_alloc<T>& lhs, const poca_alloc<U>& rhs)
{
    return lhs.imp != rhs.imp;
}

template <class S>
void test_assign(S &s1, const S& s2)
{
    try { s1 = s2; }
    catch ( std::bad_alloc &) { return; }
    assert(false);
}
#endif



template <class S>
void
test(S s1, const typename S::allocator_type& a)
{
    S s2(s1, a);
    LIBCPP_ASSERT(s2.__invariants());
    assert(s2 == s1);
    assert(s2.capacity() >= s2.size());
    assert(s2.get_allocator() == a);
}

int main()
{
    {
    typedef test_allocator<char> A;
    typedef std::basic_string<char, std::char_traits<char>, A> S;
    test(S(), A(3));
    test(S("1"), A(5));
    test(S("1234567890123456789012345678901234567890123456789012345678901234567890"), A(7));
    }
#if TEST_STD_VER >= 11
    {
    typedef min_allocator<char> A;
    typedef std::basic_string<char, std::char_traits<char>, A> S;
    test(S(), A());
    test(S("1"), A());
    test(S("1234567890123456789012345678901234567890123456789012345678901234567890"), A());
    }

#ifndef TEST_HAS_NO_EXCEPTIONS
    {
    typedef poca_alloc<char> A;
    typedef std::basic_string<char, std::char_traits<char>, A> S;
    const char * p1 = "This is my first string";
    const char * p2 = "This is my second string";

    alloc_imp<char> imp1;
    alloc_imp<char> imp2;
    S s1(p1, A(&imp1));
    S s2(p2, A(&imp2));

    assert(s1 == p1);
    assert(s2 == p2);

    imp2.deactivate();
    test_assign(s1, s2);
    assert(s1 == p1);
    assert(s2 == p2);
    }
#endif
#endif
}
