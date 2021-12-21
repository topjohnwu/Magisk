//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <typeindex>

// class type_index

// bool operator< (const type_index& rhs) const;
// bool operator<=(const type_index& rhs) const;
// bool operator> (const type_index& rhs) const;
// bool operator>=(const type_index& rhs) const;

#include <typeindex>
#include <cassert>

int main()
{
    std::type_index t1 = typeid(int);
    std::type_index t2 = typeid(int);
    std::type_index t3 = typeid(long);
    assert(!(t1 <  t2));
    assert( (t1 <= t2));
    assert(!(t1 >  t2));
    assert( (t1 >= t2));
    if (t1 < t3)
    {
        assert( (t1 <  t3));
        assert( (t1 <= t3));
        assert(!(t1 >  t3));
        assert(!(t1 >= t3));
    }
    else
    {
        assert(!(t1 <  t3));
        assert(!(t1 <= t3));
        assert( (t1 >  t3));
        assert( (t1 >= t3));
    }
}
