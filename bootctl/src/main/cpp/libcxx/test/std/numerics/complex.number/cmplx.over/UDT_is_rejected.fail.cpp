//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <complex>

// Test that UDT's convertible to an integral or floating point type do not
// participate in overload resolution.

#include <complex>
#include <type_traits>
#include <cassert>

template <class IntT>
struct UDT {
  operator IntT() const { return 1; }
};

UDT<float> ft;
UDT<double> dt;
UDT<long double> ldt;
UDT<int> it;
UDT<unsigned long> uit;

int main()
{
    {
        std::real(ft); // expected-error {{no matching function}}
        std::real(dt); // expected-error {{no matching function}}
        std::real(ldt); // expected-error {{no matching function}}
        std::real(it); // expected-error {{no matching function}}
        std::real(uit); // expected-error {{no matching function}}
    }
    {
        std::imag(ft); // expected-error {{no matching function}}
        std::imag(dt); // expected-error {{no matching function}}
        std::imag(ldt); // expected-error {{no matching function}}
        std::imag(it); // expected-error {{no matching function}}
        std::imag(uit); // expected-error {{no matching function}}
    }
    {
        std::arg(ft); // expected-error {{no matching function}}
        std::arg(dt); // expected-error {{no matching function}}
        std::arg(ldt); // expected-error {{no matching function}}
        std::arg(it); // expected-error {{no matching function}}
        std::arg(uit); // expected-error {{no matching function}}
    }
    {
        std::norm(ft); // expected-error {{no matching function}}
        std::norm(dt); // expected-error {{no matching function}}
        std::norm(ldt); // expected-error {{no matching function}}
        std::norm(it); // expected-error {{no matching function}}
        std::norm(uit); // expected-error {{no matching function}}
    }
    {
        std::conj(ft); // expected-error {{no matching function}}
        std::conj(dt); // expected-error {{no matching function}}
        std::conj(ldt); // expected-error {{no matching function}}
        std::conj(it); // expected-error {{no matching function}}
        std::conj(uit); // expected-error {{no matching function}}
    }
    {
        std::proj(ft); // expected-error {{no matching function}}
        std::proj(dt); // expected-error {{no matching function}}
        std::proj(ldt); // expected-error {{no matching function}}
        std::proj(it); // expected-error {{no matching function}}
        std::proj(uit); // expected-error {{no matching function}}
    }
}
