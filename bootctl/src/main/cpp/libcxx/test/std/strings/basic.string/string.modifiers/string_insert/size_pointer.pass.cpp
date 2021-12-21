//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// basic_string<charT,traits,Allocator>&
//   insert(size_type pos, const charT* s);

#include <string>
#include <stdexcept>
#include <cassert>

#include "test_macros.h"
#include "min_allocator.h"

template <class S>
void
test(S s, typename S::size_type pos, const typename S::value_type* str, S expected)
{
    const typename S::size_type old_size = s.size();
    S s0 = s;
    if (pos <= old_size)
    {
        s.insert(pos, str);
        LIBCPP_ASSERT(s.__invariants());
        assert(s == expected);
    }
#ifndef TEST_HAS_NO_EXCEPTIONS
    else
    {
        try
        {
            s.insert(pos, str);
            assert(false);
        }
        catch (std::out_of_range&)
        {
            assert(pos > old_size);
            assert(s == s0);
        }
    }
#endif
}

int main()
{
    {
    typedef std::string S;
    test(S(""), 0, "", S(""));
    test(S(""), 0, "12345", S("12345"));
    test(S(""), 0, "1234567890", S("1234567890"));
    test(S(""), 0, "12345678901234567890", S("12345678901234567890"));
    test(S(""), 1, "", S("can't happen"));
    test(S(""), 1, "12345", S("can't happen"));
    test(S(""), 1, "1234567890", S("can't happen"));
    test(S(""), 1, "12345678901234567890", S("can't happen"));
    test(S("abcde"), 0, "", S("abcde"));
    test(S("abcde"), 0, "12345", S("12345abcde"));
    test(S("abcde"), 0, "1234567890", S("1234567890abcde"));
    test(S("abcde"), 0, "12345678901234567890", S("12345678901234567890abcde"));
    test(S("abcde"), 1, "", S("abcde"));
    test(S("abcde"), 1, "12345", S("a12345bcde"));
    test(S("abcde"), 1, "1234567890", S("a1234567890bcde"));
    test(S("abcde"), 1, "12345678901234567890", S("a12345678901234567890bcde"));
    test(S("abcde"), 2, "", S("abcde"));
    test(S("abcde"), 2, "12345", S("ab12345cde"));
    test(S("abcde"), 2, "1234567890", S("ab1234567890cde"));
    test(S("abcde"), 2, "12345678901234567890", S("ab12345678901234567890cde"));
    test(S("abcde"), 4, "", S("abcde"));
    test(S("abcde"), 4, "12345", S("abcd12345e"));
    test(S("abcde"), 4, "1234567890", S("abcd1234567890e"));
    test(S("abcde"), 4, "12345678901234567890", S("abcd12345678901234567890e"));
    test(S("abcde"), 5, "", S("abcde"));
    test(S("abcde"), 5, "12345", S("abcde12345"));
    test(S("abcde"), 5, "1234567890", S("abcde1234567890"));
    test(S("abcde"), 5, "12345678901234567890", S("abcde12345678901234567890"));
    test(S("abcde"), 6, "", S("can't happen"));
    test(S("abcde"), 6, "12345", S("can't happen"));
    test(S("abcde"), 6, "1234567890", S("can't happen"));
    test(S("abcde"), 6, "12345678901234567890", S("can't happen"));
    test(S("abcdefghij"), 0, "", S("abcdefghij"));
    test(S("abcdefghij"), 0, "12345", S("12345abcdefghij"));
    test(S("abcdefghij"), 0, "1234567890", S("1234567890abcdefghij"));
    test(S("abcdefghij"), 0, "12345678901234567890", S("12345678901234567890abcdefghij"));
    test(S("abcdefghij"), 1, "", S("abcdefghij"));
    test(S("abcdefghij"), 1, "12345", S("a12345bcdefghij"));
    test(S("abcdefghij"), 1, "1234567890", S("a1234567890bcdefghij"));
    test(S("abcdefghij"), 1, "12345678901234567890", S("a12345678901234567890bcdefghij"));
    test(S("abcdefghij"), 5, "", S("abcdefghij"));
    test(S("abcdefghij"), 5, "12345", S("abcde12345fghij"));
    test(S("abcdefghij"), 5, "1234567890", S("abcde1234567890fghij"));
    test(S("abcdefghij"), 5, "12345678901234567890", S("abcde12345678901234567890fghij"));
    test(S("abcdefghij"), 9, "", S("abcdefghij"));
    test(S("abcdefghij"), 9, "12345", S("abcdefghi12345j"));
    test(S("abcdefghij"), 9, "1234567890", S("abcdefghi1234567890j"));
    test(S("abcdefghij"), 9, "12345678901234567890", S("abcdefghi12345678901234567890j"));
    test(S("abcdefghij"), 10, "", S("abcdefghij"));
    test(S("abcdefghij"), 10, "12345", S("abcdefghij12345"));
    test(S("abcdefghij"), 10, "1234567890", S("abcdefghij1234567890"));
    test(S("abcdefghij"), 10, "12345678901234567890", S("abcdefghij12345678901234567890"));
    test(S("abcdefghij"), 11, "", S("can't happen"));
    test(S("abcdefghij"), 11, "12345", S("can't happen"));
    test(S("abcdefghij"), 11, "1234567890", S("can't happen"));
    test(S("abcdefghij"), 11, "12345678901234567890", S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 0, "", S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, "12345", S("12345abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, "1234567890", S("1234567890abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, "12345678901234567890", S("12345678901234567890abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, "", S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, "12345", S("a12345bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, "1234567890", S("a1234567890bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, "12345678901234567890", S("a12345678901234567890bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, "", S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, "12345", S("abcdefghij12345klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, "1234567890", S("abcdefghij1234567890klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, "12345678901234567890", S("abcdefghij12345678901234567890klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 19, "", S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 19, "12345", S("abcdefghijklmnopqrs12345t"));
    test(S("abcdefghijklmnopqrst"), 19, "1234567890", S("abcdefghijklmnopqrs1234567890t"));
    test(S("abcdefghijklmnopqrst"), 19, "12345678901234567890", S("abcdefghijklmnopqrs12345678901234567890t"));
    test(S("abcdefghijklmnopqrst"), 20, "", S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 20, "12345", S("abcdefghijklmnopqrst12345"));
    test(S("abcdefghijklmnopqrst"), 20, "1234567890", S("abcdefghijklmnopqrst1234567890"));
    test(S("abcdefghijklmnopqrst"), 20, "12345678901234567890", S("abcdefghijklmnopqrst12345678901234567890"));
    test(S("abcdefghijklmnopqrst"), 21, "", S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, "12345", S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, "1234567890", S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, "12345678901234567890", S("can't happen"));
    }
#if TEST_STD_VER >= 11
    {
    typedef std::basic_string<char, std::char_traits<char>, min_allocator<char>> S;
    test(S(""), 0, "", S(""));
    test(S(""), 0, "12345", S("12345"));
    test(S(""), 0, "1234567890", S("1234567890"));
    test(S(""), 0, "12345678901234567890", S("12345678901234567890"));
    test(S(""), 1, "", S("can't happen"));
    test(S(""), 1, "12345", S("can't happen"));
    test(S(""), 1, "1234567890", S("can't happen"));
    test(S(""), 1, "12345678901234567890", S("can't happen"));
    test(S("abcde"), 0, "", S("abcde"));
    test(S("abcde"), 0, "12345", S("12345abcde"));
    test(S("abcde"), 0, "1234567890", S("1234567890abcde"));
    test(S("abcde"), 0, "12345678901234567890", S("12345678901234567890abcde"));
    test(S("abcde"), 1, "", S("abcde"));
    test(S("abcde"), 1, "12345", S("a12345bcde"));
    test(S("abcde"), 1, "1234567890", S("a1234567890bcde"));
    test(S("abcde"), 1, "12345678901234567890", S("a12345678901234567890bcde"));
    test(S("abcde"), 2, "", S("abcde"));
    test(S("abcde"), 2, "12345", S("ab12345cde"));
    test(S("abcde"), 2, "1234567890", S("ab1234567890cde"));
    test(S("abcde"), 2, "12345678901234567890", S("ab12345678901234567890cde"));
    test(S("abcde"), 4, "", S("abcde"));
    test(S("abcde"), 4, "12345", S("abcd12345e"));
    test(S("abcde"), 4, "1234567890", S("abcd1234567890e"));
    test(S("abcde"), 4, "12345678901234567890", S("abcd12345678901234567890e"));
    test(S("abcde"), 5, "", S("abcde"));
    test(S("abcde"), 5, "12345", S("abcde12345"));
    test(S("abcde"), 5, "1234567890", S("abcde1234567890"));
    test(S("abcde"), 5, "12345678901234567890", S("abcde12345678901234567890"));
    test(S("abcde"), 6, "", S("can't happen"));
    test(S("abcde"), 6, "12345", S("can't happen"));
    test(S("abcde"), 6, "1234567890", S("can't happen"));
    test(S("abcde"), 6, "12345678901234567890", S("can't happen"));
    test(S("abcdefghij"), 0, "", S("abcdefghij"));
    test(S("abcdefghij"), 0, "12345", S("12345abcdefghij"));
    test(S("abcdefghij"), 0, "1234567890", S("1234567890abcdefghij"));
    test(S("abcdefghij"), 0, "12345678901234567890", S("12345678901234567890abcdefghij"));
    test(S("abcdefghij"), 1, "", S("abcdefghij"));
    test(S("abcdefghij"), 1, "12345", S("a12345bcdefghij"));
    test(S("abcdefghij"), 1, "1234567890", S("a1234567890bcdefghij"));
    test(S("abcdefghij"), 1, "12345678901234567890", S("a12345678901234567890bcdefghij"));
    test(S("abcdefghij"), 5, "", S("abcdefghij"));
    test(S("abcdefghij"), 5, "12345", S("abcde12345fghij"));
    test(S("abcdefghij"), 5, "1234567890", S("abcde1234567890fghij"));
    test(S("abcdefghij"), 5, "12345678901234567890", S("abcde12345678901234567890fghij"));
    test(S("abcdefghij"), 9, "", S("abcdefghij"));
    test(S("abcdefghij"), 9, "12345", S("abcdefghi12345j"));
    test(S("abcdefghij"), 9, "1234567890", S("abcdefghi1234567890j"));
    test(S("abcdefghij"), 9, "12345678901234567890", S("abcdefghi12345678901234567890j"));
    test(S("abcdefghij"), 10, "", S("abcdefghij"));
    test(S("abcdefghij"), 10, "12345", S("abcdefghij12345"));
    test(S("abcdefghij"), 10, "1234567890", S("abcdefghij1234567890"));
    test(S("abcdefghij"), 10, "12345678901234567890", S("abcdefghij12345678901234567890"));
    test(S("abcdefghij"), 11, "", S("can't happen"));
    test(S("abcdefghij"), 11, "12345", S("can't happen"));
    test(S("abcdefghij"), 11, "1234567890", S("can't happen"));
    test(S("abcdefghij"), 11, "12345678901234567890", S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 0, "", S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, "12345", S("12345abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, "1234567890", S("1234567890abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 0, "12345678901234567890", S("12345678901234567890abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, "", S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, "12345", S("a12345bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, "1234567890", S("a1234567890bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 1, "12345678901234567890", S("a12345678901234567890bcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, "", S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, "12345", S("abcdefghij12345klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, "1234567890", S("abcdefghij1234567890klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 10, "12345678901234567890", S("abcdefghij12345678901234567890klmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 19, "", S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 19, "12345", S("abcdefghijklmnopqrs12345t"));
    test(S("abcdefghijklmnopqrst"), 19, "1234567890", S("abcdefghijklmnopqrs1234567890t"));
    test(S("abcdefghijklmnopqrst"), 19, "12345678901234567890", S("abcdefghijklmnopqrs12345678901234567890t"));
    test(S("abcdefghijklmnopqrst"), 20, "", S("abcdefghijklmnopqrst"));
    test(S("abcdefghijklmnopqrst"), 20, "12345", S("abcdefghijklmnopqrst12345"));
    test(S("abcdefghijklmnopqrst"), 20, "1234567890", S("abcdefghijklmnopqrst1234567890"));
    test(S("abcdefghijklmnopqrst"), 20, "12345678901234567890", S("abcdefghijklmnopqrst12345678901234567890"));
    test(S("abcdefghijklmnopqrst"), 21, "", S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, "12345", S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, "1234567890", S("can't happen"));
    test(S("abcdefghijklmnopqrst"), 21, "12345678901234567890", S("can't happen"));
    }
#endif

    { // test inserting into self
    typedef std::string S;
    S s_short = "123/";
    S s_long  = "Lorem ipsum dolor sit amet, consectetur/";

    s_short.insert(0, s_short.c_str());
    assert(s_short == "123/123/");
    s_short.insert(0, s_short.c_str());
    assert(s_short == "123/123/123/123/");
    s_short.insert(0, s_short.c_str());
    assert(s_short == "123/123/123/123/123/123/123/123/");

    s_long.insert(0, s_long.c_str());
    assert(s_long == "Lorem ipsum dolor sit amet, consectetur/Lorem ipsum dolor sit amet, consectetur/");
    }
}
