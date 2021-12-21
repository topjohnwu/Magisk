//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <memory>

// shared_ptr

// template <class T, class D>
//     bool operator==(const unique_ptr<T, D>& x, nullptr_t) noexcept;
// template <class T, class D>
//     bool operator==(nullptr_t, const unique_ptr<T, D>& y) noexcept;
// template <class T, class D>
//     bool operator!=(const unique_ptr<T, D>& x, nullptr_t) noexcept;
// template <class T, class D>
//     bool operator!=(nullptr_t, const unique_ptr<T, D>& y) noexcept;
// template <class T, class D>
//     bool operator<(const unique_ptr<T, D>& x, nullptr_t) noexcept;
// template <class T, class D>
//     bool operator<(nullptr_t, const unique_ptr<T, D>& y) noexcept;
// template <class T, class D>
//     bool operator<=(const unique_ptr<T, D>& x, nullptr_t) noexcept;
// template <class T, class D>
//     bool operator<=(nullptr_t, const unique_ptr<T, D>& y) noexcept;
// template <class T, class D>
//     bool operator>(const unique_ptr<T, D>& x, nullptr_t) noexcept;
// template <class T, class D>
//     bool operator>(nullptr_t, const unique_ptr<T, D>& y) noexcept;
// template <class T, class D>
//     bool operator>=(const unique_ptr<T, D>& x, nullptr_t) noexcept;
// template <class T, class D>
//     bool operator>=(nullptr_t, const unique_ptr<T, D>& y) noexcept;

#include <memory>
#include <cassert>

void do_nothing(int*) {}

int main()
{
    const std::unique_ptr<int> p1(new int(1));
    assert(!(p1 == nullptr));
    assert(!(nullptr == p1));
    assert(!(p1 < nullptr));
    assert( (nullptr < p1));
    assert(!(p1 <= nullptr));
    assert( (nullptr <= p1));
    assert( (p1 > nullptr));
    assert(!(nullptr > p1));
    assert( (p1 >= nullptr));
    assert(!(nullptr >= p1));

    const std::unique_ptr<int> p2;
    assert( (p2 == nullptr));
    assert( (nullptr == p2));
    assert(!(p2 < nullptr));
    assert(!(nullptr < p2));
    assert( (p2 <= nullptr));
    assert( (nullptr <= p2));
    assert(!(p2 > nullptr));
    assert(!(nullptr > p2));
    assert( (p2 >= nullptr));
    assert( (nullptr >= p2));
}
