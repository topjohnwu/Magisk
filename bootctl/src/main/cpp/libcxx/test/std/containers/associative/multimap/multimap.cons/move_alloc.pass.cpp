//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <map>

// class multimap

// multimap(multimap&& m, const allocator_type& a);

#include <map>
#include <cassert>

#include "MoveOnly.h"
#include "../../../test_compare.h"
#include "test_allocator.h"
#include "min_allocator.h"
#include "Counter.h"

int main()
{
    {
        typedef std::pair<MoveOnly, MoveOnly> V;
        typedef std::pair<const MoveOnly, MoveOnly> VC;
        typedef test_compare<std::less<MoveOnly> > C;
        typedef test_allocator<VC> A;
        typedef std::multimap<MoveOnly, MoveOnly, C, A> M;
        typedef std::move_iterator<V*> I;
        V a1[] =
        {
            V(1, 1),
            V(1, 2),
            V(1, 3),
            V(2, 1),
            V(2, 2),
            V(2, 3),
            V(3, 1),
            V(3, 2),
            V(3, 3)
        };
        M m1(I(a1), I(a1+sizeof(a1)/sizeof(a1[0])), C(5), A(7));
        V a2[] =
        {
            V(1, 1),
            V(1, 2),
            V(1, 3),
            V(2, 1),
            V(2, 2),
            V(2, 3),
            V(3, 1),
            V(3, 2),
            V(3, 3)
        };
        M m2(I(a2), I(a2+sizeof(a2)/sizeof(a2[0])), C(5), A(7));
        M m3(std::move(m1), A(7));
        assert(m3 == m2);
        assert(m3.get_allocator() == A(7));
        assert(m3.key_comp() == C(5));
        assert(m1.empty());
    }
    {
        typedef std::pair<MoveOnly, MoveOnly> V;
        typedef std::pair<const MoveOnly, MoveOnly> VC;
        typedef test_compare<std::less<MoveOnly> > C;
        typedef test_allocator<VC> A;
        typedef std::multimap<MoveOnly, MoveOnly, C, A> M;
        typedef std::move_iterator<V*> I;
        V a1[] =
        {
            V(1, 1),
            V(1, 2),
            V(1, 3),
            V(2, 1),
            V(2, 2),
            V(2, 3),
            V(3, 1),
            V(3, 2),
            V(3, 3)
        };
        M m1(I(a1), I(a1+sizeof(a1)/sizeof(a1[0])), C(5), A(7));
        V a2[] =
        {
            V(1, 1),
            V(1, 2),
            V(1, 3),
            V(2, 1),
            V(2, 2),
            V(2, 3),
            V(3, 1),
            V(3, 2),
            V(3, 3)
        };
        M m2(I(a2), I(a2+sizeof(a2)/sizeof(a2[0])), C(5), A(7));
        M m3(std::move(m1), A(5));
        assert(m3 == m2);
        assert(m3.get_allocator() == A(5));
        assert(m3.key_comp() == C(5));
        assert(m1.empty());
    }
    {
        typedef std::pair<MoveOnly, MoveOnly> V;
        typedef std::pair<const MoveOnly, MoveOnly> VC;
        typedef test_compare<std::less<MoveOnly> > C;
        typedef other_allocator<VC> A;
        typedef std::multimap<MoveOnly, MoveOnly, C, A> M;
        typedef std::move_iterator<V*> I;
        V a1[] =
        {
            V(1, 1),
            V(1, 2),
            V(1, 3),
            V(2, 1),
            V(2, 2),
            V(2, 3),
            V(3, 1),
            V(3, 2),
            V(3, 3)
        };
        M m1(I(a1), I(a1+sizeof(a1)/sizeof(a1[0])), C(5), A(7));
        V a2[] =
        {
            V(1, 1),
            V(1, 2),
            V(1, 3),
            V(2, 1),
            V(2, 2),
            V(2, 3),
            V(3, 1),
            V(3, 2),
            V(3, 3)
        };
        M m2(I(a2), I(a2+sizeof(a2)/sizeof(a2[0])), C(5), A(7));
        M m3(std::move(m1), A(5));
        assert(m3 == m2);
        assert(m3.get_allocator() == A(5));
        assert(m3.key_comp() == C(5));
        assert(m1.empty());
    }
    {
        typedef Counter<int> T;
        typedef std::pair<int, T> V;
        typedef std::pair<const int, T> VC;
        typedef test_allocator<VC> A;
        typedef std::less<int> C;
        typedef std::multimap<const int, T, C, A> M;
        typedef V* I;
        Counter_base::gConstructed = 0;
        {
            V a1[] =
            {
                V(1, 1),
                V(1, 2),
                V(1, 3),
                V(2, 1),
                V(2, 2),
                V(2, 3),
                V(3, 1),
                V(3, 2),
                V(3, 3)
            };
            const size_t num = sizeof(a1)/sizeof(a1[0]);
            assert(Counter_base::gConstructed == num);

            M m1(I(a1), I(a1+num), C(), A());
            assert(Counter_base::gConstructed == 2*num);

            M m2(m1);
            assert(m2 == m1);
            assert(Counter_base::gConstructed == 3*num);

            M m3(std::move(m1), A());
            assert(m3 == m2);
            assert(m1.empty());
            assert(Counter_base::gConstructed == 3*num);

            {
            M m4(std::move(m2), A(5));
            assert(Counter_base::gConstructed == 3*num);
            assert(m4 == m3);
            assert(m2.empty());
            }
            assert(Counter_base::gConstructed == 2*num);
        }
        assert(Counter_base::gConstructed == 0);
    }
    {
        typedef std::pair<MoveOnly, MoveOnly> V;
        typedef std::pair<const MoveOnly, MoveOnly> VC;
        typedef test_compare<std::less<MoveOnly> > C;
        typedef min_allocator<VC> A;
        typedef std::multimap<MoveOnly, MoveOnly, C, A> M;
        typedef std::move_iterator<V*> I;
        V a1[] =
        {
            V(1, 1),
            V(1, 2),
            V(1, 3),
            V(2, 1),
            V(2, 2),
            V(2, 3),
            V(3, 1),
            V(3, 2),
            V(3, 3)
        };
        M m1(I(a1), I(a1+sizeof(a1)/sizeof(a1[0])), C(5), A());
        V a2[] =
        {
            V(1, 1),
            V(1, 2),
            V(1, 3),
            V(2, 1),
            V(2, 2),
            V(2, 3),
            V(3, 1),
            V(3, 2),
            V(3, 3)
        };
        M m2(I(a2), I(a2+sizeof(a2)/sizeof(a2[0])), C(5), A());
        M m3(std::move(m1), A());
        assert(m3 == m2);
        assert(m3.get_allocator() == A());
        assert(m3.key_comp() == C(5));
        assert(m1.empty());
    }
    {
        typedef std::pair<MoveOnly, MoveOnly> V;
        typedef std::pair<const MoveOnly, MoveOnly> VC;
        typedef test_compare<std::less<MoveOnly> > C;
        typedef explicit_allocator<VC> A;
        typedef std::multimap<MoveOnly, MoveOnly, C, A> M;
        typedef std::move_iterator<V*> I;
        V a1[] =
        {
            V(1, 1),
            V(1, 2),
            V(1, 3),
            V(2, 1),
            V(2, 2),
            V(2, 3),
            V(3, 1),
            V(3, 2),
            V(3, 3)
        };
        M m1(I(a1), I(a1+sizeof(a1)/sizeof(a1[0])), C(5), A{});
        V a2[] =
        {
            V(1, 1),
            V(1, 2),
            V(1, 3),
            V(2, 1),
            V(2, 2),
            V(2, 3),
            V(3, 1),
            V(3, 2),
            V(3, 3)
        };
        M m2(I(a2), I(a2+sizeof(a2)/sizeof(a2[0])), C(5), A{});
        M m3(std::move(m1), A{});
        assert(m3 == m2);
        assert(m3.get_allocator() == A{});
        assert(m3.key_comp() == C(5));
        assert(m1.empty());
    }
}
