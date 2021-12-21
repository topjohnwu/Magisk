#ifndef CHECK_H_
#define CHECK_H_

#include <cmath>
#include <cstdlib>
#include <ostream>

#include "internal_macros.h"
#include "log.h"

namespace benchmark {
namespace internal {

typedef void(AbortHandlerT)();

inline AbortHandlerT*& GetAbortHandler() {
  static AbortHandlerT* handler = &std::abort;
  return handler;
}

BENCHMARK_NORETURN inline void CallAbortHandler() {
  GetAbortHandler()();
  std::abort();  // fallback to enforce noreturn
}

// CheckHandler is the class constructed by failing CHECK macros. CheckHandler
// will log information about the failures and abort when it is destructed.
class CheckHandler {
 public:
  CheckHandler(const char* check, const char* file, const char* func, int line)
      : log_(GetErrorLogInstance()) {
    log_ << file << ":" << line << ": " << func << ": Check `" << check
         << "' failed. ";
  }

  LogType& GetLog() { return log_; }

  BENCHMARK_NORETURN ~CheckHandler() BENCHMARK_NOEXCEPT_OP(false) {
    log_ << std::endl;
    CallAbortHandler();
  }

  CheckHandler& operator=(const CheckHandler&) = delete;
  CheckHandler(const CheckHandler&) = delete;
  CheckHandler() = delete;

 private:
  LogType& log_;
};

}  // end namespace internal
}  // end namespace benchmark

// The CHECK macro returns a std::ostream object that can have extra information
// written to it.
#ifndef NDEBUG
#define CHECK(b)                                                             \
  (b ? ::benchmark::internal::GetNullLogInstance()                           \
     : ::benchmark::internal::CheckHandler(#b, __FILE__, __func__, __LINE__) \
           .GetLog())
#else
#define CHECK(b) ::benchmark::internal::GetNullLogInstance()
#endif

// clang-format off
// preserve whitespacing between operators for alignment
#define CHECK_EQ(a, b) CHECK((a) == (b))
#define CHECK_NE(a, b) CHECK((a) != (b))
#define CHECK_GE(a, b) CHECK((a) >= (b))
#define CHECK_LE(a, b) CHECK((a) <= (b))
#define CHECK_GT(a, b) CHECK((a) > (b))
#define CHECK_LT(a, b) CHECK((a) < (b))

#define CHECK_FLOAT_EQ(a, b, eps) CHECK(std::fabs((a) - (b)) <  (eps))
#define CHECK_FLOAT_NE(a, b, eps) CHECK(std::fabs((a) - (b)) >= (eps))
#define CHECK_FLOAT_GE(a, b, eps) CHECK((a) - (b) > -(eps))
#define CHECK_FLOAT_LE(a, b, eps) CHECK((b) - (a) > -(eps))
#define CHECK_FLOAT_GT(a, b, eps) CHECK((a) - (b) >  (eps))
#define CHECK_FLOAT_LT(a, b, eps) CHECK((b) - (a) >  (eps))
//clang-format on

#endif  // CHECK_H_
