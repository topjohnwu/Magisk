//===--- StringView.h -------------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//
// This file is copied from llvm/lib/Demangle/StringView.h.
//===----------------------------------------------------------------------===//

#ifndef LIBCXX_DEMANGLE_STRINGVIEW_H
#define LIBCXX_DEMANGLE_STRINGVIEW_H

#include <algorithm>
#include <cassert>
#include <cstring>

namespace {
class StringView {
  const char *First;
  const char *Last;

public:
  template <size_t N>
  StringView(const char (&Str)[N]) : First(Str), Last(Str + N - 1) {}
  StringView(const char *First_, const char *Last_)
      : First(First_), Last(Last_) {}
  StringView(const char *First_, size_t Len)
      : First(First_), Last(First_ + Len) {}
  StringView(const char *Str) : First(Str), Last(Str + std::strlen(Str)) {}
  StringView() : First(nullptr), Last(nullptr) {}

  StringView substr(size_t From) const {
    return StringView(begin() + From, size() - From);
  }

  StringView substr(size_t From, size_t To) const {
    if (To >= size())
      To = size() - 1;
    if (From >= size())
      From = size() - 1;
    return StringView(First + From, First + To);
  }

  StringView dropFront(size_t N = 1) const {
    if (N >= size())
      N = size() - 1;
    return StringView(First + N, Last);
  }

  char front() const {
    assert(!empty());
    return *begin();
  }

  char popFront() {
    assert(!empty());
    return *First++;
  }

  bool consumeFront(char C) {
    if (!startsWith(C))
      return false;
    *this = dropFront(1);
    return true;
  }

  bool consumeFront(StringView S) {
    if (!startsWith(S))
      return false;
    *this = dropFront(S.size());
    return true;
  }

  bool startsWith(char C) const { return !empty() && *begin() == C; }

  bool startsWith(StringView Str) const {
    if (Str.size() > size())
      return false;
    return std::equal(Str.begin(), Str.end(), begin());
  }

  const char &operator[](size_t Idx) const { return *(begin() + Idx); }

  const char *begin() const { return First; }
  const char *end() const { return Last; }
  size_t size() const { return static_cast<size_t>(Last - First); }
  bool empty() const { return First == Last; }
};

inline bool operator==(const StringView &LHS, const StringView &RHS) {
  return LHS.size() == RHS.size() &&
         std::equal(LHS.begin(), LHS.end(), RHS.begin());
}
} // namespace

#endif
