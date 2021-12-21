//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// Not a portable test

// __scan_keyword
// Scans [__b, __e) until a match is found in the basic_strings range
//  [__kb, __ke) or until it can be shown that there is no match in [__kb, __ke).
//  __b will be incremented (visibly), consuming CharT until a match is found
//  or proved to not exist.  A keyword may be "", in which will match anything.
//  If one keyword is a prefix of another, and the next CharT in the input
//  might match another keyword, the algorithm will attempt to find the longest
//  matching keyword.  If the longer matching keyword ends up not matching, then
//  no keyword match is found.  If no keyword match is found, __ke is returned.
//  Else an iterator pointing to the matching keyword is found.  If more than
//  one keyword matches, an iterator to the first matching keyword is returned.
//  If on exit __b == __e, eofbit is set in __err.  If __case_sensitive is false,
//  __ct is used to force to lower case before comparing characters.
//  Examples:
//  Keywords:  "a", "abb"
//  If the input is "a", the first keyword matches and eofbit is set.
//  If the input is "abc", no match is found and "ab" are consumed.
//
// template <class _InputIterator, class _ForwardIterator, class _Ctype>
// _ForwardIterator
// __scan_keyword(_InputIterator& __b, _InputIterator __e,
//                _ForwardIterator __kb, _ForwardIterator __ke,
//                const _Ctype& __ct, ios_base::iostate& __err,
//                bool __case_sensitive = true);

#include <locale>
#include <cassert>

int main()
{
    const std::ctype<char>& ct = std::use_facet<std::ctype<char> >(std::locale::classic());
    std::ios_base::iostate err = std::ios_base::goodbit;
    {
        const char input[] = "a";
        const char* in = input;
        std::string keys[] = {"a", "abb"};
        err = std::ios_base::goodbit;
        std::string* k = std::__scan_keyword(in, input+sizeof(input)-1,
                                             keys, keys+sizeof(keys)/sizeof(keys[0]),
                                             ct, err);
        assert(k - keys == 0);
        assert(in == input+1);
        assert(err == std::ios_base::eofbit);
    }
    {
        const char input[] = "abc";
        const char* in = input;
        std::string keys[] = {"a", "abb"};
        err = std::ios_base::goodbit;
        std::string* k = std::__scan_keyword(in, input+sizeof(input)-1,
                                             keys, keys+sizeof(keys)/sizeof(keys[0]),
                                             ct, err);
        assert(k - keys == 2);
        assert(in == input+2);
        assert(err == std::ios_base::failbit);
    }
    {
        const char input[] = "abb";
        const char* in = input;
        std::string keys[] = {"a", "abb"};
        err = std::ios_base::goodbit;
        std::string* k = std::__scan_keyword(in, input+sizeof(input)-1,
                                             keys, keys+sizeof(keys)/sizeof(keys[0]),
                                             ct, err);
        assert(k - keys == 1);
        assert(in == input+3);
        assert(err == std::ios_base::eofbit);
    }
    {
        const char input[] = "Tue ";
        const char* in = input;
        std::string keys[] = {"Mon", "Monday", "Tue", "Tuesday"};
        err = std::ios_base::goodbit;
        std::string* k = std::__scan_keyword(in, input+sizeof(input)-1,
                                             keys, keys+sizeof(keys)/sizeof(keys[0]),
                                             ct, err);
        assert(k - keys == 2);
        assert(in == input+3);
        assert(err == std::ios_base::goodbit);
    }
    {
        const char input[] = "tue ";
        const char* in = input;
        std::string keys[] = {"Mon", "Monday", "Tue", "Tuesday"};
        err = std::ios_base::goodbit;
        std::string* k = std::__scan_keyword(in, input+sizeof(input)-1,
                                             keys, keys+sizeof(keys)/sizeof(keys[0]),
                                             ct, err);
        assert(k - keys == 4);
        assert(in == input+0);
        assert(err == std::ios_base::failbit);
    }
    {
        const char input[] = "tue ";
        const char* in = input;
        std::string keys[] = {"Mon", "Monday", "Tue", "Tuesday"};
        err = std::ios_base::goodbit;
        std::string* k = std::__scan_keyword(in, input+sizeof(input)-1,
                                             keys, keys+sizeof(keys)/sizeof(keys[0]),
                                             ct, err, false);
        assert(k - keys == 2);
        assert(in == input+3);
        assert(err == std::ios_base::goodbit);
    }
}
