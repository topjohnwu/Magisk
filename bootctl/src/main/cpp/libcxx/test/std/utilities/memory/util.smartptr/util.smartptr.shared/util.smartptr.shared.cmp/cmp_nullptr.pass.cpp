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

// template <class T>
//     bool operator==(const shared_ptr<T>& x, nullptr_t) noexcept;
// template <class T>
//     bool operator==(nullptr_t, const shared_ptr<T>& y) noexcept;
// template <class T>
//     bool operator!=(const shared_ptr<T>& x, nullptr_t) noexcept;
// template <class T>
//     bool operator!=(nullptr_t, const shared_ptr<T>& y) noexcept;
// template <class T>
//     bool operator<(const shared_ptr<T>& x, nullptr_t) noexcept;
// template <class T>
//     bool operator<(nullptr_t, const shared_ptr<T>& y) noexcept;
// template <class T>
//     bool operator<=(const shared_ptr<T>& x, nullptr_t) noexcept;
// template <class T>
//     bool operator<=(nullptr_t, const shared_ptr<T>& y) noexcept;
// template <class T>
//     bool operator>(const shared_ptr<T>& x, nullptr_t) noexcept;
// template <class T>
//     bool operator>(nullptr_t, const shared_ptr<T>& y) noexcept;
// template <class T>
//     bool operator>=(const shared_ptr<T>& x, nullptr_t) noexcept;
// template <class T>
//     bool operator>=(nullptr_t, const shared_ptr<T>& y) noexcept;

#include <memory>
#include <cassert>

void do_nothing(int*) {}

int main()
{
    const std::shared_ptr<int> p1(new int(1));
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

    const std::shared_ptr<int> p2;
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
