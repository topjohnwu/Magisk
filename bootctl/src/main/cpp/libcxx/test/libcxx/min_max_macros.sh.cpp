// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Test that we can include each header in two TU's and link them together.

// RUN: %compile -fsyntax-only

// Prevent <ext/hash_map> from generating deprecated warnings for this test.
#if defined(__DEPRECATED)
#undef __DEPRECATED
#endif

#define TEST_MACROS() static_assert(min() == true && max() == true, "")
#define min() true
#define max() true

// Top level headers
#include <algorithm>
TEST_MACROS();
#include <any>
TEST_MACROS();
#include <array>
TEST_MACROS();
#ifndef _LIBCPP_HAS_NO_THREADS
#include <atomic>
TEST_MACROS();
#endif
#include <bitset>
TEST_MACROS();
#include <cassert>
TEST_MACROS();
#include <ccomplex>
TEST_MACROS();
#include <cctype>
TEST_MACROS();
#include <cerrno>
TEST_MACROS();
#include <cfenv>
TEST_MACROS();
#include <cfloat>
TEST_MACROS();
#include <charconv>
TEST_MACROS();
#include <chrono>
TEST_MACROS();
#include <cinttypes>
TEST_MACROS();
#include <ciso646>
TEST_MACROS();
#include <climits>
TEST_MACROS();
#include <clocale>
TEST_MACROS();
#include <cmath>
TEST_MACROS();
#include <codecvt>
TEST_MACROS();
#include <complex>
TEST_MACROS();
#include <complex.h>
TEST_MACROS();
#include <condition_variable>
TEST_MACROS();
#include <csetjmp>
TEST_MACROS();
#include <csignal>
TEST_MACROS();
#include <cstdarg>
TEST_MACROS();
#include <cstdbool>
TEST_MACROS();
#include <cstddef>
TEST_MACROS();
#include <cstdint>
TEST_MACROS();
#include <cstdio>
TEST_MACROS();
#include <cstdlib>
TEST_MACROS();
#include <cstring>
TEST_MACROS();
#include <ctgmath>
TEST_MACROS();
#include <ctime>
TEST_MACROS();
#include <ctype.h>
TEST_MACROS();
#include <cwchar>
TEST_MACROS();
#include <cwctype>
TEST_MACROS();
#include <deque>
TEST_MACROS();
#include <errno.h>
TEST_MACROS();
#include <exception>
TEST_MACROS();
#include <float.h>
TEST_MACROS();
#include <forward_list>
TEST_MACROS();
#include <fstream>
TEST_MACROS();
#include <functional>
TEST_MACROS();
#ifndef _LIBCPP_HAS_NO_THREADS
#include <future>
TEST_MACROS();
#endif
#include <initializer_list>
TEST_MACROS();
#include <inttypes.h>
TEST_MACROS();
#include <iomanip>
TEST_MACROS();
#include <ios>
TEST_MACROS();
#include <iosfwd>
TEST_MACROS();
#include <iostream>
TEST_MACROS();
#include <istream>
TEST_MACROS();
#include <iterator>
TEST_MACROS();
#include <limits>
TEST_MACROS();
#include <limits.h>
TEST_MACROS();
#include <list>
TEST_MACROS();
#include <locale>
TEST_MACROS();
#include <locale.h>
TEST_MACROS();
#include <map>
TEST_MACROS();
#include <math.h>
TEST_MACROS();
#include <memory>
TEST_MACROS();
#ifndef _LIBCPP_HAS_NO_THREADS
#include <mutex>
TEST_MACROS();
#endif
#include <new>
TEST_MACROS();
#include <numeric>
TEST_MACROS();
#include <optional>
TEST_MACROS();
#include <ostream>
TEST_MACROS();
#include <queue>
TEST_MACROS();
#include <random>
TEST_MACROS();
#include <ratio>
TEST_MACROS();
#include <regex>
TEST_MACROS();
#include <scoped_allocator>
TEST_MACROS();
#include <set>
TEST_MACROS();
#include <setjmp.h>
TEST_MACROS();
#ifndef _LIBCPP_HAS_NO_THREADS
#include <shared_mutex>
TEST_MACROS();
#endif
#include <sstream>
TEST_MACROS();
#include <stack>
TEST_MACROS();
#include <stdbool.h>
TEST_MACROS();
#include <stddef.h>
TEST_MACROS();
#include <stdexcept>
TEST_MACROS();
#include <stdint.h>
TEST_MACROS();
#include <stdio.h>
TEST_MACROS();
#include <stdlib.h>
TEST_MACROS();
#include <streambuf>
TEST_MACROS();
#include <string>
TEST_MACROS();
#include <string.h>
TEST_MACROS();
#include <string_view>
TEST_MACROS();
#include <strstream>
TEST_MACROS();
#include <system_error>
TEST_MACROS();
#include <tgmath.h>
TEST_MACROS();
#ifndef _LIBCPP_HAS_NO_THREADS
#include <thread>
TEST_MACROS();
#endif
#include <tuple>
TEST_MACROS();
#include <typeindex>
TEST_MACROS();
#include <typeinfo>
TEST_MACROS();
#include <type_traits>
TEST_MACROS();
#include <unordered_map>
TEST_MACROS();
#include <unordered_set>
TEST_MACROS();
#include <utility>
TEST_MACROS();
#include <valarray>
TEST_MACROS();
#include <variant>
TEST_MACROS();
#include <vector>
TEST_MACROS();
#include <wchar.h>
TEST_MACROS();
#include <wctype.h>
TEST_MACROS();

// experimental headers
#if __cplusplus >= 201103L
#include <experimental/algorithm>
TEST_MACROS();
#include <experimental/deque>
TEST_MACROS();
#include <experimental/filesystem>
TEST_MACROS();
#include <experimental/forward_list>
TEST_MACROS();
#include <experimental/functional>
TEST_MACROS();
#include <experimental/iterator>
TEST_MACROS();
#include <experimental/list>
TEST_MACROS();
#include <experimental/map>
TEST_MACROS();
#include <experimental/memory_resource>
TEST_MACROS();
#include <experimental/propagate_const>
TEST_MACROS();
#include <experimental/regex>
TEST_MACROS();
#include <experimental/set>
TEST_MACROS();
#include <experimental/string>
TEST_MACROS();
#include <experimental/type_traits>
TEST_MACROS();
#include <experimental/unordered_map>
TEST_MACROS();
#include <experimental/unordered_set>
TEST_MACROS();
#include <experimental/utility>
TEST_MACROS();
#include <experimental/vector>
TEST_MACROS();
#endif // __cplusplus >= 201103L

// extended headers
#include <ext/hash_map>
TEST_MACROS();
#include <ext/hash_set>
TEST_MACROS();
