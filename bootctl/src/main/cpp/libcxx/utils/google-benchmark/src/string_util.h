#ifndef BENCHMARK_STRING_UTIL_H_
#define BENCHMARK_STRING_UTIL_H_

#include <sstream>
#include <string>
#include <utility>
#include "internal_macros.h"

namespace benchmark {

void AppendHumanReadable(int n, std::string* str);

std::string HumanReadableNumber(double n, double one_k = 1024.0);

#ifdef __GNUC__
__attribute__((format(printf, 1, 2)))
#endif
std::string
StrFormat(const char* format, ...);

inline std::ostream& StrCatImp(std::ostream& out) BENCHMARK_NOEXCEPT {
  return out;
}

template <class First, class... Rest>
inline std::ostream& StrCatImp(std::ostream& out, First&& f, Rest&&... rest) {
  out << std::forward<First>(f);
  return StrCatImp(out, std::forward<Rest>(rest)...);
}

template <class... Args>
inline std::string StrCat(Args&&... args) {
  std::ostringstream ss;
  StrCatImp(ss, std::forward<Args>(args)...);
  return ss.str();
}

void ReplaceAll(std::string* str, const std::string& from,
                const std::string& to);

#ifdef BENCHMARK_STL_ANDROID_GNUSTL
/*
 * GNU STL in Android NDK lacks support for some C++11 functions, including
 * stoul, stoi, stod. We reimplement them here using C functions strtoul,
 * strtol, strtod. Note that reimplemented functions are in benchmark::
 * namespace, not std:: namespace.
 */
unsigned long stoul(const std::string& str, size_t* pos = nullptr,
                           int base = 10);
int stoi(const std::string& str, size_t* pos = nullptr, int base = 10);
double stod(const std::string& str, size_t* pos = nullptr);
#else
using std::stoul;
using std::stoi;
using std::stod;
#endif

}  // end namespace benchmark

#endif  // BENCHMARK_STRING_UTIL_H_
