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

// pattern pos_format() const;

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
        std::money_base::pattern p = f.pos_format();
        assert(p.field[0] == std::money_base::symbol);
        assert(p.field[1] == std::money_base::sign);
        assert(p.field[2] == std::money_base::none);
        assert(p.field[3] == std::money_base::value);
    }
    {
        Fnt f(1);
        std::money_base::pattern p = f.pos_format();
        assert(p.field[0] == std::money_base::symbol);
        assert(p.field[1] == std::money_base::sign);
        assert(p.field[2] == std::money_base::none);
        assert(p.field[3] == std::money_base::value);
    }
    {
        Fwf f(1);
        std::money_base::pattern p = f.pos_format();
        assert(p.field[0] == std::money_base::symbol);
        assert(p.field[1] == std::money_base::sign);
        assert(p.field[2] == std::money_base::none);
        assert(p.field[3] == std::money_base::value);
    }
    {
        Fwt f(1);
        std::money_base::pattern p = f.pos_format();
        assert(p.field[0] == std::money_base::symbol);
        assert(p.field[1] == std::money_base::sign);
        assert(p.field[2] == std::money_base::none);
        assert(p.field[3] == std::money_base::value);
    }
}
