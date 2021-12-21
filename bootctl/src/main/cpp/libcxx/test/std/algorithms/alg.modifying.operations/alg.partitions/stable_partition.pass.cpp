//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>

// template<BidirectionalIterator Iter, Predicate<auto, Iter::value_type> Pred>
//   requires ShuffleIterator<Iter>
//         && CopyConstructible<Pred>
//   Iter
//   stable_partition(Iter first, Iter last, Pred pred);

#include <algorithm>
#include <cassert>
#include <memory>

#include "test_macros.h"
#include "test_iterators.h"

struct is_odd
{
    bool operator()(const int& i) const {return i & 1;}
};

struct odd_first
{
    bool operator()(const std::pair<int,int>& p) const
        {return p.first & 1;}
};

template <class Iter>
void
test()
{
    {  // check mixed
    typedef std::pair<int,int> P;
    P array[] =
    {
        P(0, 1),
        P(0, 2),
        P(1, 1),
        P(1, 2),
        P(2, 1),
        P(2, 2),
        P(3, 1),
        P(3, 2),
        P(4, 1),
        P(4, 2)
    };
    const unsigned size = sizeof(array)/sizeof(array[0]);
    Iter r = std::stable_partition(Iter(array), Iter(array+size), odd_first());
    assert(base(r) == array + 4);
    assert(array[0] == P(1, 1));
    assert(array[1] == P(1, 2));
    assert(array[2] == P(3, 1));
    assert(array[3] == P(3, 2));
    assert(array[4] == P(0, 1));
    assert(array[5] == P(0, 2));
    assert(array[6] == P(2, 1));
    assert(array[7] == P(2, 2));
    assert(array[8] == P(4, 1));
    assert(array[9] == P(4, 2));
    }
    {
    typedef std::pair<int,int> P;
    P array[] =
    {
        P(0, 1),
        P(0, 2),
        P(1, 1),
        P(1, 2),
        P(2, 1),
        P(2, 2),
        P(3, 1),
        P(3, 2),
        P(4, 1),
        P(4, 2)
    };
    const unsigned size = sizeof(array)/sizeof(array[0]);
    Iter r = std::stable_partition(Iter(array), Iter(array+size), odd_first());
    assert(base(r) == array + 4);
    assert(array[0] == P(1, 1));
    assert(array[1] == P(1, 2));
    assert(array[2] == P(3, 1));
    assert(array[3] == P(3, 2));
    assert(array[4] == P(0, 1));
    assert(array[5] == P(0, 2));
    assert(array[6] == P(2, 1));
    assert(array[7] == P(2, 2));
    assert(array[8] == P(4, 1));
    assert(array[9] == P(4, 2));
    // check empty
    r = std::stable_partition(Iter(array), Iter(array), odd_first());
    assert(base(r) == array);
    // check one true
    r = std::stable_partition(Iter(array), Iter(array+1), odd_first());
    assert(base(r) == array+1);
    assert(array[0] == P(1, 1));
    // check one false
    r = std::stable_partition(Iter(array+4), Iter(array+5), odd_first());
    assert(base(r) == array+4);
    assert(array[4] == P(0, 1));
    }
    {  // check all false
    typedef std::pair<int,int> P;
    P array[] =
    {
        P(0, 1),
        P(0, 2),
        P(2, 1),
        P(2, 2),
        P(4, 1),
        P(4, 2),
        P(6, 1),
        P(6, 2),
        P(8, 1),
        P(8, 2)
    };
    const unsigned size = sizeof(array)/sizeof(array[0]);
    Iter r = std::stable_partition(Iter(array), Iter(array+size), odd_first());
    assert(base(r) == array);
    assert(array[0] == P(0, 1));
    assert(array[1] == P(0, 2));
    assert(array[2] == P(2, 1));
    assert(array[3] == P(2, 2));
    assert(array[4] == P(4, 1));
    assert(array[5] == P(4, 2));
    assert(array[6] == P(6, 1));
    assert(array[7] == P(6, 2));
    assert(array[8] == P(8, 1));
    assert(array[9] == P(8, 2));
    }
    {  // check all true
    typedef std::pair<int,int> P;
    P array[] =
    {
        P(1, 1),
        P(1, 2),
        P(3, 1),
        P(3, 2),
        P(5, 1),
        P(5, 2),
        P(7, 1),
        P(7, 2),
        P(9, 1),
        P(9, 2)
    };
    const unsigned size = sizeof(array)/sizeof(array[0]);
    Iter r = std::stable_partition(Iter(array), Iter(array+size), odd_first());
    assert(base(r) == array + size);
    assert(array[0] == P(1, 1));
    assert(array[1] == P(1, 2));
    assert(array[2] == P(3, 1));
    assert(array[3] == P(3, 2));
    assert(array[4] == P(5, 1));
    assert(array[5] == P(5, 2));
    assert(array[6] == P(7, 1));
    assert(array[7] == P(7, 2));
    assert(array[8] == P(9, 1));
    assert(array[9] == P(9, 2));
    }
    {  // check all false but first true
    typedef std::pair<int,int> P;
    P array[] =
    {
        P(1, 1),
        P(0, 2),
        P(2, 1),
        P(2, 2),
        P(4, 1),
        P(4, 2),
        P(6, 1),
        P(6, 2),
        P(8, 1),
        P(8, 2)
    };
    const unsigned size = sizeof(array)/sizeof(array[0]);
    Iter r = std::stable_partition(Iter(array), Iter(array+size), odd_first());
    assert(base(r) == array + 1);
    assert(array[0] == P(1, 1));
    assert(array[1] == P(0, 2));
    assert(array[2] == P(2, 1));
    assert(array[3] == P(2, 2));
    assert(array[4] == P(4, 1));
    assert(array[5] == P(4, 2));
    assert(array[6] == P(6, 1));
    assert(array[7] == P(6, 2));
    assert(array[8] == P(8, 1));
    assert(array[9] == P(8, 2));
    }
    {  // check all false but last true
    typedef std::pair<int,int> P;
    P array[] =
    {
        P(0, 1),
        P(0, 2),
        P(2, 1),
        P(2, 2),
        P(4, 1),
        P(4, 2),
        P(6, 1),
        P(6, 2),
        P(8, 1),
        P(1, 2)
    };
    const unsigned size = sizeof(array)/sizeof(array[0]);
    Iter r = std::stable_partition(Iter(array), Iter(array+size), odd_first());
    assert(base(r) == array + 1);
    assert(array[0] == P(1, 2));
    assert(array[1] == P(0, 1));
    assert(array[2] == P(0, 2));
    assert(array[3] == P(2, 1));
    assert(array[4] == P(2, 2));
    assert(array[5] == P(4, 1));
    assert(array[6] == P(4, 2));
    assert(array[7] == P(6, 1));
    assert(array[8] == P(6, 2));
    assert(array[9] == P(8, 1));
    }
    {  // check all true but first false
    typedef std::pair<int,int> P;
    P array[] =
    {
        P(0, 1),
        P(1, 2),
        P(3, 1),
        P(3, 2),
        P(5, 1),
        P(5, 2),
        P(7, 1),
        P(7, 2),
        P(9, 1),
        P(9, 2)
    };
    const unsigned size = sizeof(array)/sizeof(array[0]);
    Iter r = std::stable_partition(Iter(array), Iter(array+size), odd_first());
    assert(base(r) == array + size-1);
    assert(array[0] == P(1, 2));
    assert(array[1] == P(3, 1));
    assert(array[2] == P(3, 2));
    assert(array[3] == P(5, 1));
    assert(array[4] == P(5, 2));
    assert(array[5] == P(7, 1));
    assert(array[6] == P(7, 2));
    assert(array[7] == P(9, 1));
    assert(array[8] == P(9, 2));
    assert(array[9] == P(0, 1));
    }
    {  // check all true but last false
    typedef std::pair<int,int> P;
    P array[] =
    {
        P(1, 1),
        P(1, 2),
        P(3, 1),
        P(3, 2),
        P(5, 1),
        P(5, 2),
        P(7, 1),
        P(7, 2),
        P(9, 1),
        P(0, 2)
    };
    const unsigned size = sizeof(array)/sizeof(array[0]);
    Iter r = std::stable_partition(Iter(array), Iter(array+size), odd_first());
    assert(base(r) == array + size-1);
    assert(array[0] == P(1, 1));
    assert(array[1] == P(1, 2));
    assert(array[2] == P(3, 1));
    assert(array[3] == P(3, 2));
    assert(array[4] == P(5, 1));
    assert(array[5] == P(5, 2));
    assert(array[6] == P(7, 1));
    assert(array[7] == P(7, 2));
    assert(array[8] == P(9, 1));
    assert(array[9] == P(0, 2));
    }
}

#if TEST_STD_VER >= 11

struct is_null
{
    template <class P>
        bool operator()(const P& p) {return p == 0;}
};

template <class Iter>
void
test1()
{
    const unsigned size = 5;
    std::unique_ptr<int> array[size];
    Iter r = std::stable_partition(Iter(array), Iter(array+size), is_null());
    assert(r == Iter(array+size));
}

#endif  // TEST_STD_VER >= 11

int main()
{
    test<bidirectional_iterator<std::pair<int,int>*> >();
    test<random_access_iterator<std::pair<int,int>*> >();
    test<std::pair<int,int>*>();

#if TEST_STD_VER >= 11
    test1<bidirectional_iterator<std::unique_ptr<int>*> >();
#endif
}
