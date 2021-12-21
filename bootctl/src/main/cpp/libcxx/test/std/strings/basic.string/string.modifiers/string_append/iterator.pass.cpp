//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// template<class InputIterator>
//   basic_string& append(InputIterator first, InputIterator last);

#include <string>
#include <cassert>

#include "test_iterators.h"
#include "min_allocator.h"

template <class S, class It>
void
test(S s, It first, It last, S expected)
{
    s.append(first, last);
    LIBCPP_ASSERT(s.__invariants());
    assert(s == expected);
}

#ifndef TEST_HAS_NO_EXCEPTIONS
template <class S, class It>
void
test_exceptions(S s, It first, It last)
{
    S aCopy = s;
    try {
        s.append(first, last);
        assert(false);
        }
    catch (...) {}
    LIBCPP_ASSERT(s.__invariants());
    assert(s == aCopy);
}
#endif

int main()
{
    {
    typedef std::string S;
    const char* s = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    test(S(), s, s, S());
    test(S(), s, s+1, S("A"));
    test(S(), s, s+10, S("ABCDEFGHIJ"));
    test(S(), s, s+52, S("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"));

    test(S("12345"), s, s, S("12345"));
    test(S("12345"), s, s+1, S("12345A"));
    test(S("12345"), s, s+10, S("12345ABCDEFGHIJ"));
    test(S("12345"), s, s+52, S("12345ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"));

    test(S("1234567890"), s, s, S("1234567890"));
    test(S("1234567890"), s, s+1, S("1234567890A"));
    test(S("1234567890"), s, s+10, S("1234567890ABCDEFGHIJ"));
    test(S("1234567890"), s, s+52, S("1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"));

    test(S("12345678901234567890"), s, s, S("12345678901234567890"));
    test(S("12345678901234567890"), s, s+1, S("12345678901234567890""A"));
    test(S("12345678901234567890"), s, s+10, S("12345678901234567890""ABCDEFGHIJ"));
    test(S("12345678901234567890"), s, s+52,
         S("12345678901234567890""ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"));

    test(S(), input_iterator<const char*>(s), input_iterator<const char*>(s), S());
    test(S(), input_iterator<const char*>(s), input_iterator<const char*>(s+1), S("A"));
    test(S(), input_iterator<const char*>(s), input_iterator<const char*>(s+10),
         S("ABCDEFGHIJ"));
    test(S(), input_iterator<const char*>(s), input_iterator<const char*>(s+52),
         S("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"));

    test(S("12345"), input_iterator<const char*>(s), input_iterator<const char*>(s),
         S("12345"));
    test(S("12345"), input_iterator<const char*>(s), input_iterator<const char*>(s+1),
         S("12345A"));
    test(S("12345"), input_iterator<const char*>(s), input_iterator<const char*>(s+10),
         S("12345ABCDEFGHIJ"));
    test(S("12345"), input_iterator<const char*>(s), input_iterator<const char*>(s+52),
         S("12345ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"));

    test(S("1234567890"), input_iterator<const char*>(s), input_iterator<const char*>(s),
         S("1234567890"));
    test(S("1234567890"), input_iterator<const char*>(s), input_iterator<const char*>(s+1),
         S("1234567890A"));
    test(S("1234567890"), input_iterator<const char*>(s), input_iterator<const char*>(s+10),
         S("1234567890ABCDEFGHIJ"));
    test(S("1234567890"), input_iterator<const char*>(s), input_iterator<const char*>(s+52),
         S("1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"));

    test(S("12345678901234567890"), input_iterator<const char*>(s), input_iterator<const char*>(s),
         S("12345678901234567890"));
    test(S("12345678901234567890"), input_iterator<const char*>(s), input_iterator<const char*>(s+1),
         S("12345678901234567890""A"));
    test(S("12345678901234567890"), input_iterator<const char*>(s), input_iterator<const char*>(s+10),
         S("12345678901234567890""ABCDEFGHIJ"));
    test(S("12345678901234567890"), input_iterator<const char*>(s), input_iterator<const char*>(s+52),
         S("12345678901234567890""ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"));
    }
#if TEST_STD_VER >= 11
    {
    typedef std::basic_string<char, std::char_traits<char>, min_allocator<char>> S;
    const char* s = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    test(S(), s, s, S());
    test(S(), s, s+1, S("A"));
    test(S(), s, s+10, S("ABCDEFGHIJ"));
    test(S(), s, s+52, S("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"));

    test(S("12345"), s, s, S("12345"));
    test(S("12345"), s, s+1, S("12345A"));
    test(S("12345"), s, s+10, S("12345ABCDEFGHIJ"));
    test(S("12345"), s, s+52, S("12345ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"));

    test(S("1234567890"), s, s, S("1234567890"));
    test(S("1234567890"), s, s+1, S("1234567890A"));
    test(S("1234567890"), s, s+10, S("1234567890ABCDEFGHIJ"));
    test(S("1234567890"), s, s+52, S("1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"));

    test(S("12345678901234567890"), s, s, S("12345678901234567890"));
    test(S("12345678901234567890"), s, s+1, S("12345678901234567890""A"));
    test(S("12345678901234567890"), s, s+10, S("12345678901234567890""ABCDEFGHIJ"));
    test(S("12345678901234567890"), s, s+52,
         S("12345678901234567890""ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"));

    test(S(), input_iterator<const char*>(s), input_iterator<const char*>(s), S());
    test(S(), input_iterator<const char*>(s), input_iterator<const char*>(s+1), S("A"));
    test(S(), input_iterator<const char*>(s), input_iterator<const char*>(s+10),
         S("ABCDEFGHIJ"));
    test(S(), input_iterator<const char*>(s), input_iterator<const char*>(s+52),
         S("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"));

    test(S("12345"), input_iterator<const char*>(s), input_iterator<const char*>(s),
         S("12345"));
    test(S("12345"), input_iterator<const char*>(s), input_iterator<const char*>(s+1),
         S("12345A"));
    test(S("12345"), input_iterator<const char*>(s), input_iterator<const char*>(s+10),
         S("12345ABCDEFGHIJ"));
    test(S("12345"), input_iterator<const char*>(s), input_iterator<const char*>(s+52),
         S("12345ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"));

    test(S("1234567890"), input_iterator<const char*>(s), input_iterator<const char*>(s),
         S("1234567890"));
    test(S("1234567890"), input_iterator<const char*>(s), input_iterator<const char*>(s+1),
         S("1234567890A"));
    test(S("1234567890"), input_iterator<const char*>(s), input_iterator<const char*>(s+10),
         S("1234567890ABCDEFGHIJ"));
    test(S("1234567890"), input_iterator<const char*>(s), input_iterator<const char*>(s+52),
         S("1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"));

    test(S("12345678901234567890"), input_iterator<const char*>(s), input_iterator<const char*>(s),
         S("12345678901234567890"));
    test(S("12345678901234567890"), input_iterator<const char*>(s), input_iterator<const char*>(s+1),
         S("12345678901234567890""A"));
    test(S("12345678901234567890"), input_iterator<const char*>(s), input_iterator<const char*>(s+10),
         S("12345678901234567890""ABCDEFGHIJ"));
    test(S("12345678901234567890"), input_iterator<const char*>(s), input_iterator<const char*>(s+52),
         S("12345678901234567890""ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"));
    }
#endif
#ifndef TEST_HAS_NO_EXCEPTIONS
    { // test iterator operations that throw
    typedef std::string S;
    typedef ThrowingIterator<char> TIter;
    typedef input_iterator<TIter> IIter;
    const char* s = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    test_exceptions(S(), IIter(TIter(s, s+10, 4, TIter::TAIncrement)), IIter());
    test_exceptions(S(), IIter(TIter(s, s+10, 5, TIter::TADereference)), IIter());
    test_exceptions(S(), IIter(TIter(s, s+10, 6, TIter::TAComparison)), IIter());

    test_exceptions(S(), TIter(s, s+10, 4, TIter::TAIncrement), TIter());
    test_exceptions(S(), TIter(s, s+10, 5, TIter::TADereference), TIter());
    test_exceptions(S(), TIter(s, s+10, 6, TIter::TAComparison), TIter());
    }
#endif

    { // test appending to self
    typedef std::string S;
    S s_short = "123/";
    S s_long  = "Lorem ipsum dolor sit amet, consectetur/";

    s_short.append(s_short.begin(), s_short.end());
    assert(s_short == "123/123/");
    s_short.append(s_short.begin(), s_short.end());
    assert(s_short == "123/123/123/123/");
    s_short.append(s_short.begin(), s_short.end());
    assert(s_short == "123/123/123/123/123/123/123/123/");

    s_long.append(s_long.begin(), s_long.end());
    assert(s_long == "Lorem ipsum dolor sit amet, consectetur/Lorem ipsum dolor sit amet, consectetur/");
    }

    { // test appending a different type
    typedef std::string S;
    const uint8_t p[] = "ABCD";

    S s;
    s.append(p, p + 4);
    assert(s == "ABCD");
    }

  { // test with a move iterator that returns char&&
    typedef forward_iterator<const char*> It;
    typedef std::move_iterator<It> MoveIt;
    const char p[] = "ABCD";
    std::string s;
    s.append(MoveIt(It(std::begin(p))), MoveIt(It(std::end(p) - 1)));
    assert(s == "ABCD");
  }
  { // test with a move iterator that returns char&&
    typedef const char* It;
    typedef std::move_iterator<It> MoveIt;
    const char p[] = "ABCD";
    std::string s;
    s.append(MoveIt(It(std::begin(p))), MoveIt(It(std::end(p) - 1)));
    assert(s == "ABCD");
  }
}
