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
//               ios_base::iostate& err, void*& v) const;

#include <locale>
#include <ios>
#include <cassert>
#include <streambuf>
#include "test_iterators.h"

typedef std::num_get<char, input_iterator<const char*> > F;

class my_facet
    : public F
{
public:
    explicit my_facet(std::size_t refs = 0)
        : F(refs) {}
};

int main()
{
    const my_facet f(1);
    std::ios ios(0);
    {
        const char str[] = "0x0";
        std::ios_base::iostate err = ios.goodbit;
        void* p;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+sizeof(str)),
                  ios, err, p);
        assert(iter.base() == str+sizeof(str)-1);
        assert(err == ios.goodbit);
        assert(p == 0);
    }
    {
        const char str[] = "0x73";
        std::ios_base::iostate err = ios.goodbit;
        void* p;
        input_iterator<const char*> iter =
            f.get(input_iterator<const char*>(str),
                  input_iterator<const char*>(str+sizeof(str)),
                  ios, err, p);
        assert(iter.base() == str+sizeof(str)-1);
        assert(err == ios.goodbit);
        assert(p == (void*)0x73);
    }
}
