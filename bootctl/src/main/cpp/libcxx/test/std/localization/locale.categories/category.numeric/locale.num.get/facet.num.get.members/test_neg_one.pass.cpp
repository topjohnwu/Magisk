//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// class num_get<charT, InputIterator>

// iter_type get(iter_type in, iter_type end, ios_base&,
//               ios_base::iostate& err, unsigned int& v) const;

#include <limits>
#include <locale>
#include <ios>
#include <cassert>
#include <streambuf>
#include <sstream>
#include <iostream>
#include "test_iterators.h"
#include "test_macros.h"

#ifdef TEST_COMPILER_C1XX
#pragma warning(disable: 4146) // unary minus operator applied to unsigned type, result still unsigned
#endif

typedef std::num_get<char, input_iterator<const char*> > F;

class my_facet
    : public F
{
public:
    explicit my_facet(std::size_t refs = 0)
        : F(refs) {}
};

template <class T>
std::string make_neg_string(T value) {
  std::ostringstream ss;
  assert(ss << value);
  std::string res = ss.str();
  return '-' + res;
}

template <class T>
void test_neg_one() {
    const my_facet f(1);
    std::ios ios(0);
    T v = static_cast<T>(42);
    {
        const char str[] = "-1";
        std::ios_base::iostate err = ios.goodbit;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+sizeof(str)),
                  ios, err, v);
        assert(iter.base() == str+sizeof(str)-1);
        assert(err == ios.goodbit);
        assert(v == T(-1));
    }
    v = 42;
    {
        const char str[] = "-";
        std::ios_base::iostate err = ios.goodbit;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+sizeof(str)),
                  ios, err, v);
        assert(iter.base() == str+sizeof(str)-1);
        assert(err == ios.failbit);
        assert(v == 0);
    }
}

template <class T>
void test_negate() {
    typedef typename std::make_signed<T>::type SignedT;
    const my_facet f(1);
    std::ios ios(0);
    T v = 42;
    {
        T value = std::numeric_limits<SignedT>::max();
        ++value;
        std::string std_str = make_neg_string(value);
        const char* str = std_str.data();
        size_t size = std_str.size();
        std::ios_base::iostate err = ios.goodbit;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+size+1),
                  ios, err, v);
        assert(iter.base() == str+size);
        assert(err == ios.goodbit);
        T expected = -value;
        assert(v == expected);
    }
    v = 42;
    {
        T value = std::numeric_limits<SignedT>::max();
        ++value;
        ++value;
        std::string std_str = make_neg_string(value);
        const char* str = std_str.data();
        size_t size = std_str.size();
        std::ios_base::iostate err = ios.goodbit;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+size+1),
                  ios, err, v);
        assert(iter.base() == str+size);
        assert(err == ios.goodbit);
        T expected = -value;
        assert(v == expected);
    }
    v = 42;
    {
        T value = std::numeric_limits<T>::max();
        std::string std_str = make_neg_string(value);
        const char* str = std_str.data();
        size_t size = std_str.size();
        std::ios_base::iostate err = ios.goodbit;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+size+1),
                  ios, err, v);
        assert(iter.base() == str+size);
        assert(err == ios.goodbit);
        T expected = -value;
        assert(v == expected);
    }
    v = 42;
    {
        std::string std_str = make_neg_string(std::numeric_limits<T>::max());
        std_str.back()++;
        const char* str = std_str.data();
        size_t size = std_str.size();
        std::ios_base::iostate err = ios.goodbit;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+size+1),
                  ios, err, v);
        assert(iter.base() == str+size);
        assert(err == ios.failbit);
        assert(v == T(-1));
    }
}

int main()
{
    test_neg_one<long>();
    test_neg_one<long long>();
    test_neg_one<unsigned short>();
    test_neg_one<unsigned int>();
    test_neg_one<unsigned long>();
    test_neg_one<unsigned long long>();

    test_negate<unsigned short>();
    test_negate<unsigned int>();
    test_negate<unsigned long>();
    test_negate<unsigned long long>();
}
