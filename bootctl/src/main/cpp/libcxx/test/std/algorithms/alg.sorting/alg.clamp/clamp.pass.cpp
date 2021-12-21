//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>
// XFAIL: c++98, c++03, c++11, c++14

// template<class T>
//   const T&
//   clamp(const T& v, const T& lo, const T& hi);

#include <algorithm>
#include <cassert>

struct Tag {
    Tag() : val(0), tag("Default") {}
    Tag(int a, const char *b) : val(a), tag(b) {}
    ~Tag() {}

    int val;
    const char *tag;
    };

bool eq(const Tag& rhs, const Tag& lhs) { return rhs.val == lhs.val && rhs.tag == lhs.tag; }
// bool operator==(const Tag& rhs, const Tag& lhs) { return rhs.val == lhs.val; }
bool operator< (const Tag& rhs, const Tag& lhs) { return rhs.val <  lhs.val; }

template <class T>
void
test(const T& a, const T& lo, const T& hi, const T& x)
{
    assert(&std::clamp(a, lo, hi) == &x);
}

int main()
{
    {
    int x = 0;
    int y = 0;
    int z = 0;
    test(x, y, z, x);
    test(y, x, z, y);
    }
    {
    int x = 0;
    int y = 1;
    int z = 2;
    test(x, y, z, y);
    test(y, x, z, y);
    }
    {
    int x = 1;
    int y = 0;
    int z = 1;
    test(x, y, z, x);
    test(y, x, z, x);
    }

    {
//  If they're all the same, we should get the value back.
    Tag x{0, "Zero-x"};
    Tag y{0, "Zero-y"};
    Tag z{0, "Zero-z"};
    assert(eq(std::clamp(x, y, z), x));
    assert(eq(std::clamp(y, x, z), y));
    }

    {
//  If it's the same as the lower bound, we get the value back.
    Tag x{0, "Zero-x"};
    Tag y{0, "Zero-y"};
    Tag z{1, "One-z"};
    assert(eq(std::clamp(x, y, z), x));
    assert(eq(std::clamp(y, x, z), y));
    }

    {
//  If it's the same as the upper bound, we get the value back.
    Tag x{1, "One-x"};
    Tag y{0, "Zero-y"};
    Tag z{1, "One-z"};
    assert(eq(std::clamp(x, y, z), x));
    assert(eq(std::clamp(z, y, x), z));
    }

    {
//  If the value is between, we should get the value back
    Tag x{1, "One-x"};
    Tag y{0, "Zero-y"};
    Tag z{2, "Two-z"};
    assert(eq(std::clamp(x, y, z), x));
    assert(eq(std::clamp(y, x, z), x));
    }

    {
//  If the value is less than the 'lo', we should get the lo back.
    Tag x{0, "Zero-x"};
    Tag y{1, "One-y"};
    Tag z{2, "Two-z"};
    assert(eq(std::clamp(x, y, z), y));
    assert(eq(std::clamp(y, x, z), y));
    }
    {
//  If the value is greater than 'hi', we should get hi back.
    Tag x{2, "Two-x"};
    Tag y{0, "Zero-y"};
    Tag z{1, "One-z"};
    assert(eq(std::clamp(x, y, z), z));
    assert(eq(std::clamp(y, z, x), z));
    }

    {
    typedef int T;
    constexpr T x = 1;
    constexpr T y = 0;
    constexpr T z = 1;
    static_assert(std::clamp(x, y, z) == x, "" );
    static_assert(std::clamp(y, x, z) == x, "" );
    }
}
