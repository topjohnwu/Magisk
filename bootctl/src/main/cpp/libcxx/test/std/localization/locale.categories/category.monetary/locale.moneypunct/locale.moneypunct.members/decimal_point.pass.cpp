//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// class moneypunct<charT, International>

// charT decimal_point() const;

// The C++ and C standards are silent.
//   POSIX standard is being followed (as a guideline).

#include <locale>
#include <limits>
#include <cassert>

typedef std::moneypunct<char> F;

class Fnf
    : public std::moneypunct<char, false>
{
public:
    explicit Fnf(std::size_t refs = 0)
        : std::moneypunct<char, false>(refs) {}
};

class Fnt
    : public std::moneypunct<char, true>
{
public:
    explicit Fnt(std::size_t refs = 0)
        : std::moneypunct<char, true>(refs) {}
};

class Fwf
    : public std::moneypunct<wchar_t, false>
{
public:
    explicit Fwf(std::size_t refs = 0)
        : std::moneypunct<wchar_t, false>(refs) {}
};

class Fwt
    : public std::moneypunct<wchar_t, true>
{
public:
    explicit Fwt(std::size_t refs = 0)
        : std::moneypunct<wchar_t, true>(refs) {}
};

int main()
{
    {
        Fnf f(1);
        assert(f.decimal_point() == std::numeric_limits<char>::max());
    }
    {
        Fnt f(1);
        assert(f.decimal_point() == std::numeric_limits<char>::max());
    }
    {
        Fwf f(1);
        assert(f.decimal_point() == std::numeric_limits<wchar_t>::max());
    }
    {
        Fwt f(1);
        assert(f.decimal_point() == std::numeric_limits<wchar_t>::max());
    }
}
