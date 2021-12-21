//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>

// template<class ForwardIterator1, class ForwardIterator2, class BinaryPredicate>
//   constexpr bool   // constexpr after C++17
//   is_permutation(ForwardIterator1 first1, ForwardIterator1 last1,
//                  ForwardIterator2 first2, BinaryPredicate pred);

#include <algorithm>
#include <functional>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"

int comparison_count = 0;
template <typename T>
bool counting_equals ( const T &a, const T &b ) {
    ++comparison_count;
    return a == b;
    }

#if TEST_STD_VER > 17
constexpr bool test_constexpr() {
    int ia[] = {0, 0, 0};
    int ib[] = {1, 1, 0};
    int ic[] = {1, 0, 1};
    int id[] = {1};
    std::equal_to<int> c{};
    return !std::is_permutation(std::begin(ia), std::end(ia), std::begin(ib)              , c)
        && !std::is_permutation(std::begin(ia), std::end(ia), std::begin(ib), std::end(ib), c)
        &&  std::is_permutation(std::begin(ib), std::end(ib), std::begin(ic)              , c)
        &&  std::is_permutation(std::begin(ib), std::end(ib), std::begin(ic), std::end(ic), c)
        && !std::is_permutation(std::begin(ic), std::end(ic), std::begin(id), std::end(id), c)
        ;
    }
#endif


struct S {
  S(int i) : i_(i) {}
  bool operator==(const S& other) = delete;
  int i_;
};

struct eq {
  bool operator()(const S& a, const S&b) { return a.i_ == b.i_; }
};


int main()
{
    {
        const int ia[] = {0};
        const int ib[] = {0};
        const unsigned sa = sizeof(ia)/sizeof(ia[0]);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + 0),
                                   forward_iterator<const int*>(ib),
                                   std::equal_to<const int>()) == true);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   std::equal_to<const int>()) == true);
#if TEST_STD_VER >= 14
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa),
                                   std::equal_to<const int>()) == true);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa - 1),
                                   std::equal_to<const int>()) == false);
#endif
    }
    {
        const int ia[] = {0};
        const int ib[] = {1};
        const unsigned sa = sizeof(ia)/sizeof(ia[0]);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   std::equal_to<const int>()) == false);
#if TEST_STD_VER >= 14
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa),
                                   std::equal_to<const int>()) == false);
#endif
    }

    {
        const int ia[] = {0, 0};
        const int ib[] = {0, 0};
        const unsigned sa = sizeof(ia)/sizeof(ia[0]);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   std::equal_to<const int>()) == true);
#if TEST_STD_VER >= 14
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa),
                                   std::equal_to<const int>()) == true);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa - 1),
                                   std::equal_to<const int>()) == false);
#endif
    }
    {
        const int ia[] = {0, 0};
        const int ib[] = {0, 1};
        const unsigned sa = sizeof(ia)/sizeof(ia[0]);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   std::equal_to<const int>()) == false);
#if TEST_STD_VER >= 14
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa),
                                   std::equal_to<const int>()) == false);
#endif
    }
    {
        const int ia[] = {0, 0};
        const int ib[] = {1, 0};
        const unsigned sa = sizeof(ia)/sizeof(ia[0]);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   std::equal_to<const int>()) == false);
#if TEST_STD_VER >= 14
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa),
                                   std::equal_to<const int>()) == false);
#endif
    }
    {
        const int ia[] = {0, 0};
        const int ib[] = {1, 1};
        const unsigned sa = sizeof(ia)/sizeof(ia[0]);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   std::equal_to<const int>()) == false);
#if TEST_STD_VER >= 14
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa),
                                   std::equal_to<const int>()) == false);
#endif
    }
    {
        const int ia[] = {0, 1};
        const int ib[] = {0, 0};
        const unsigned sa = sizeof(ia)/sizeof(ia[0]);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   std::equal_to<const int>()) == false);
#if TEST_STD_VER >= 14
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa),
                                   std::equal_to<const int>()) == false);
#endif
    }
    {
        const int ia[] = {0, 1};
        const int ib[] = {0, 1};
        const unsigned sa = sizeof(ia)/sizeof(ia[0]);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   std::equal_to<const int>()) == true);
#if TEST_STD_VER >= 14
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa),
                                   std::equal_to<const int>()) == true);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa - 1),
                                   std::equal_to<const int>()) == false);
#endif
    }
    {
        const int ia[] = {0, 1};
        const int ib[] = {1, 0};
        const unsigned sa = sizeof(ia)/sizeof(ia[0]);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   std::equal_to<const int>()) == true);
#if TEST_STD_VER >= 14
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa),
                                   std::equal_to<const int>()) == true);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa - 1),
                                   std::equal_to<const int>()) == false);
#endif
    }
    {
        const int ia[] = {0, 1};
        const int ib[] = {1, 1};
        const unsigned sa = sizeof(ia)/sizeof(ia[0]);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   std::equal_to<const int>()) == false);
#if TEST_STD_VER >= 14
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa),
                                   std::equal_to<const int>()) == false);
#endif
    }
    {
        const int ia[] = {1, 0};
        const int ib[] = {0, 0};
        const unsigned sa = sizeof(ia)/sizeof(ia[0]);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   std::equal_to<const int>()) == false);
#if TEST_STD_VER >= 14
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa),
                                   std::equal_to<const int>()) == false);
#endif
    }
    {
        const int ia[] = {1, 0};
        const int ib[] = {0, 1};
        const unsigned sa = sizeof(ia)/sizeof(ia[0]);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   std::equal_to<const int>()) == true);
#if TEST_STD_VER >= 14
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa),
                                   std::equal_to<const int>()) == true);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa - 1),
                                   std::equal_to<const int>()) == false);
#endif
    }
    {
        const int ia[] = {1, 0};
        const int ib[] = {1, 0};
        const unsigned sa = sizeof(ia)/sizeof(ia[0]);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   std::equal_to<const int>()) == true);
#if TEST_STD_VER >= 14
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa),
                                   std::equal_to<const int>()) == true);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa - 1),
                                   std::equal_to<const int>()) == false);
#endif
    }
    {
        const int ia[] = {1, 0};
        const int ib[] = {1, 1};
        const unsigned sa = sizeof(ia)/sizeof(ia[0]);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   std::equal_to<const int>()) == false);
#if TEST_STD_VER >= 14
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa),
                                   std::equal_to<const int>()) == false);
#endif
    }
    {
        const int ia[] = {1, 1};
        const int ib[] = {0, 0};
        const unsigned sa = sizeof(ia)/sizeof(ia[0]);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   std::equal_to<const int>()) == false);
#if TEST_STD_VER >= 14
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa),
                                   std::equal_to<const int>()) == false);
#endif
    }
    {
        const int ia[] = {1, 1};
        const int ib[] = {0, 1};
        const unsigned sa = sizeof(ia)/sizeof(ia[0]);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   std::equal_to<const int>()) == false);
#if TEST_STD_VER >= 14
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa),
                                   std::equal_to<const int>()) == false);
#endif
    }
    {
        const int ia[] = {1, 1};
        const int ib[] = {1, 0};
        const unsigned sa = sizeof(ia)/sizeof(ia[0]);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   std::equal_to<const int>()) == false);
#if TEST_STD_VER >= 14
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa),
                                   std::equal_to<const int>()) == false);
#endif
    }
    {
        const int ia[] = {1, 1};
        const int ib[] = {1, 1};
        const unsigned sa = sizeof(ia)/sizeof(ia[0]);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   std::equal_to<const int>()) == true);
#if TEST_STD_VER >= 14
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa),
                                   std::equal_to<const int>()) == true);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa - 1),
                                   std::equal_to<const int>()) == false);
#endif
    }

    {
        const int ia[] = {0, 0, 0};
        const int ib[] = {1, 0, 0};
        const unsigned sa = sizeof(ia)/sizeof(ia[0]);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   std::equal_to<const int>()) == false);
#if TEST_STD_VER >= 14
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa),
                                   std::equal_to<const int>()) == false);
#endif
    }
    {
        const int ia[] = {0, 0, 0};
        const int ib[] = {1, 0, 1};
        const unsigned sa = sizeof(ia)/sizeof(ia[0]);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   std::equal_to<const int>()) == false);
#if TEST_STD_VER >= 14
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa),
                                   std::equal_to<const int>()) == false);
#endif
    }
    {
        const int ia[] = {0, 0, 0};
        const int ib[] = {1, 0, 2};
        const unsigned sa = sizeof(ia)/sizeof(ia[0]);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   std::equal_to<const int>()) == false);
#if TEST_STD_VER >= 14
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa),
                                   std::equal_to<const int>()) == false);
#endif
    }
    {
        const int ia[] = {0, 0, 0};
        const int ib[] = {1, 1, 0};
        const unsigned sa = sizeof(ia)/sizeof(ia[0]);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   std::equal_to<const int>()) == false);
#if TEST_STD_VER >= 14
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa),
                                   std::equal_to<const int>()) == false);
#endif
    }
    {
        const int ia[] = {0, 0, 0};
        const int ib[] = {1, 1, 1};
        const unsigned sa = sizeof(ia)/sizeof(ia[0]);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   std::equal_to<const int>()) == false);
#if TEST_STD_VER >= 14
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa),
                                   std::equal_to<const int>()) == false);
#endif
    }
    {
        const int ia[] = {0, 0, 0};
        const int ib[] = {1, 1, 2};
        const unsigned sa = sizeof(ia)/sizeof(ia[0]);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   std::equal_to<const int>()) == false);
#if TEST_STD_VER >= 14
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa),
                                   std::equal_to<const int>()) == false);
#endif
    }
    {
        const int ia[] = {0, 0, 0};
        const int ib[] = {1, 2, 0};
        const unsigned sa = sizeof(ia)/sizeof(ia[0]);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   std::equal_to<const int>()) == false);
#if TEST_STD_VER >= 14
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa),
                                   std::equal_to<const int>()) == false);
#endif
    }
    {
        const int ia[] = {0, 0, 0};
        const int ib[] = {1, 2, 1};
        const unsigned sa = sizeof(ia)/sizeof(ia[0]);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   std::equal_to<const int>()) == false);
#if TEST_STD_VER >= 14
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa),
                                   std::equal_to<const int>()) == false);
#endif
    }
    {
        const int ia[] = {0, 0, 0};
        const int ib[] = {1, 2, 2};
        const unsigned sa = sizeof(ia)/sizeof(ia[0]);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   std::equal_to<const int>()) == false);
#if TEST_STD_VER >= 14
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa),
                                   std::equal_to<const int>()) == false);
#endif
    }
    {
        const int ia[] = {0, 0, 1};
        const int ib[] = {1, 0, 0};
        const unsigned sa = sizeof(ia)/sizeof(ia[0]);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   std::equal_to<const int>()) == true);
#if TEST_STD_VER >= 14
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa),
                                   std::equal_to<const int>()) == true);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa - 1),
                                   std::equal_to<const int>()) == false);
#endif
    }
    {
        const int ia[] = {0, 0, 1};
        const int ib[] = {1, 0, 1};
        const unsigned sa = sizeof(ia)/sizeof(ia[0]);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   std::equal_to<const int>()) == false);
#if TEST_STD_VER >= 14
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa),
                                   std::equal_to<const int>()) == false);
#endif
    }
    {
        const int ia[] = {0, 1, 2};
        const int ib[] = {1, 0, 2};
        const unsigned sa = sizeof(ia)/sizeof(ia[0]);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   std::equal_to<const int>()) == true);
#if TEST_STD_VER >= 14
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa),
                                   std::equal_to<const int>()) == true);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa - 1),
                                   std::equal_to<const int>()) == false);
#endif
    }
    {
        const int ia[] = {0, 1, 2};
        const int ib[] = {1, 2, 0};
        const unsigned sa = sizeof(ia)/sizeof(ia[0]);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   std::equal_to<const int>()) == true);
#if TEST_STD_VER >= 14
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa),
                                   std::equal_to<const int>()) == true);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa - 1),
                                   std::equal_to<const int>()) == false);
#endif
    }
    {
        const int ia[] = {0, 1, 2};
        const int ib[] = {2, 1, 0};
        const unsigned sa = sizeof(ia)/sizeof(ia[0]);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   std::equal_to<const int>()) == true);
#if TEST_STD_VER >= 14
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa),
                                   std::equal_to<const int>()) == true);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa - 1),
                                   std::equal_to<const int>()) == false);
#endif
    }
    {
        const int ia[] = {0, 1, 2};
        const int ib[] = {2, 0, 1};
        const unsigned sa = sizeof(ia)/sizeof(ia[0]);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   std::equal_to<const int>()) == true);
#if TEST_STD_VER >= 14
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa),
                                   std::equal_to<const int>()) == true);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa - 1),
                                   std::equal_to<const int>()) == false);
#endif
    }
    {
        const int ia[] = {0, 0, 1};
        const int ib[] = {1, 0, 1};
        const unsigned sa = sizeof(ia)/sizeof(ia[0]);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   std::equal_to<const int>()) == false);
#if TEST_STD_VER >= 14
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa),
                                   std::equal_to<const int>()) == false);
#endif
    }
    {
        const int ia[] = {0, 0, 1};
        const int ib[] = {1, 0, 0};
        const unsigned sa = sizeof(ia)/sizeof(ia[0]);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   std::equal_to<const int>()) == true);
#if TEST_STD_VER >= 14
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa),
                                   std::equal_to<const int>()) == true);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib + 1),
                                   forward_iterator<const int*>(ib + sa),
                                   std::equal_to<const int>()) == false);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa - 1),
                                   std::equal_to<const int>()) == false);
#endif
    }
    {
        const int ia[] = {0, 1, 2, 3, 0, 5, 6, 2, 4, 4};
        const int ib[] = {4, 2, 3, 0, 1, 4, 0, 5, 6, 2};
        const unsigned sa = sizeof(ia)/sizeof(ia[0]);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   std::equal_to<const int>()) == true);
#if TEST_STD_VER >= 14
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa),
                                   std::equal_to<const int>()) == true);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib + 1),
                                   forward_iterator<const int*>(ib + sa),
                                   std::equal_to<const int>()) == false);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa - 1),
                                   std::equal_to<const int>()) == false);
        comparison_count = 0;
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa - 1),
                                   counting_equals<const int>) == false);
        assert ( comparison_count > 0 );
        comparison_count = 0;
        assert(std::is_permutation(random_access_iterator<const int*>(ia),
                                   random_access_iterator<const int*>(ia + sa),
                                   random_access_iterator<const int*>(ib),
                                   random_access_iterator<const int*>(ib + sa - 1),
                                   counting_equals<const int>) == false);
        assert ( comparison_count == 0 );
#endif
    }
    {
        const int ia[] = {0, 1, 2, 3, 0, 5, 6, 2, 4, 4};
        const int ib[] = {4, 2, 3, 0, 1, 4, 0, 5, 6, 0};
        const unsigned sa = sizeof(ia)/sizeof(ia[0]);
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   std::equal_to<const int>()) == false);
#if TEST_STD_VER >= 14
        assert(std::is_permutation(forward_iterator<const int*>(ia),
                                   forward_iterator<const int*>(ia + sa),
                                   forward_iterator<const int*>(ib),
                                   forward_iterator<const int*>(ib + sa),
                                   std::equal_to<const int>()) == false);
#endif
    }
    {
      const S a[] = {S(0), S(1)};
      const S b[] = {S(1), S(0)};
      const unsigned sa = sizeof(a)/sizeof(a[0]);
      assert(std::is_permutation(forward_iterator<const S*>(a),
                                 forward_iterator<const S*>(a + sa),
                                 forward_iterator<const S*>(b),
                                 eq()));
#if TEST_STD_VER >= 14
      assert(std::is_permutation(forward_iterator<const S*>(a),
                                 forward_iterator<const S*>(a + sa),
                                 forward_iterator<const S*>(b),
                                 forward_iterator<const S*>(b + sa),
                                 eq()));
#endif
    }

#if TEST_STD_VER > 17
    static_assert(test_constexpr());
#endif
}
