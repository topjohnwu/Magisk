//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>
//
// template <class T>
// struct unwrap_ref_decay;
//
// template <class T>
// using unwrap_ref_decay_t = typename unwrap_ref_decay<T>::type;

// UNSUPPORTED: c++98, c++03, c++11, c++14, c++17

#include <functional>
#include <type_traits>


template <typename T, typename Result>
void check() {
  static_assert(std::is_same_v<typename std::unwrap_ref_decay<T>::type, Result>);
  static_assert(std::is_same_v<typename std::unwrap_ref_decay<T>::type, std::unwrap_ref_decay_t<T>>);
}

struct T { };

int main() {
  check<T,             T>();
  check<T&,            T>();
  check<T const,       T>();
  check<T const&,      T>();
  check<T*,            T*>();
  check<T const*,      T const*>();
  check<T[3],          T*>();
  check<T const [3],   T const*>();
  check<T (),          T (*)()>();
  check<T (int) const, T (int) const>();
  check<T (int) &,     T (int) &>();
  check<T (int) &&,    T (int) &&>();

  check<std::reference_wrapper<T>,         T&>();
  check<std::reference_wrapper<T>&,        T&>();
  check<std::reference_wrapper<T const>,   T const&>();
  check<std::reference_wrapper<T const>&,  T const&>();
  check<std::reference_wrapper<T*>,        T*&>();
  check<std::reference_wrapper<T*>&,       T*&>();
  check<std::reference_wrapper<T const*>,  T const*&>();
  check<std::reference_wrapper<T const*>&, T const*&>();
  check<std::reference_wrapper<T[3]>,      T (&)[3]>();
  check<std::reference_wrapper<T[3]>&,     T (&)[3]>();
  check<std::reference_wrapper<T ()>,      T (&)()>();
  check<std::reference_wrapper<T ()>&,     T (&)()>();
}
