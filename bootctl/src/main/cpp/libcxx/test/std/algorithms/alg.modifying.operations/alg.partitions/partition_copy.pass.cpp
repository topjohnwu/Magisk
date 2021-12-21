//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>

// template <class InputIterator, class OutputIterator1,
//           class OutputIterator2, class Predicate>
//     constexpr pair<OutputIterator1, OutputIterator2>     // constexpr after C++17
//     partition_copy(InputIterator first, InputIterator last,
//                    OutputIterator1 out_true, OutputIterator2 out_false,
//                    Predicate pred);

#include <algorithm>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"

struct is_odd
{
    TEST_CONSTEXPR bool operator()(const int& i) const {return i & 1;}
};

#if TEST_STD_VER > 17
TEST_CONSTEXPR bool test_constexpr() {
    int ia[] = {1, 3, 5, 2, 4, 6};
    int r1[10] = {0};
    int r2[10] = {0};

    auto p = std::partition_copy(std::begin(ia), std::end(ia),
                    std::begin(r1), std::begin(r2), is_odd());

    return std::all_of(std::begin(r1), p.first, is_odd())
        && std::all_of(p.first, std::end(r1), [](int a){return a == 0;})
        && std::none_of(std::begin(r2), p.second, is_odd())
        && std::all_of(p.second, std::end(r2), [](int a){return a == 0;})
           ;
    }
#endif

int main()
{
    {
        const int ia[] = {1, 2, 3, 4, 6, 8, 5, 7};
        int r1[10] = {0};
        int r2[10] = {0};
        typedef std::pair<output_iterator<int*>,  int*> P;
        P p = std::partition_copy(input_iterator<const int*>(std::begin(ia)),
                                  input_iterator<const int*>(std::end(ia)),
                                  output_iterator<int*>(r1), r2, is_odd());
        assert(p.first.base() == r1 + 4);
        assert(r1[0] == 1);
        assert(r1[1] == 3);
        assert(r1[2] == 5);
        assert(r1[3] == 7);
        assert(p.second == r2 + 4);
        assert(r2[0] == 2);
        assert(r2[1] == 4);
        assert(r2[2] == 6);
        assert(r2[3] == 8);
    }

#if TEST_STD_VER > 17
    static_assert(test_constexpr());
#endif
}
