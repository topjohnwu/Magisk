#ifndef TEST_SUPPORT_VERBOSE_ASSERT
#define TEST_SUPPORT_VERBOSE_ASSERT

#include <iostream>
#include <cstdio>
#include <sstream>
#include <string>
#include "test_macros.h"

namespace verbose_assert {

typedef std::basic_ostream<char>&(EndLType)(std::basic_ostream<char>&);

template <class Stream, class Tp,
    class = decltype(std::declval<Stream&>() << std::declval<Tp const&>())>
std::true_type IsStreamableImp(int);
template <class Stream, class Tp> std::false_type IsStreamableImp(long);

template <class Stream, class Tp>
struct IsStreamable : decltype(IsStreamableImp<Stream, Tp>(0)) {};

template <class Tp, int ST = (IsStreamable<decltype(std::cerr), Tp>::value ? 1
        : (IsStreamable<decltype(std::wcerr), Tp>::value ? 2 : -1))>
struct SelectStream {
  static_assert(ST == -1, "specialization required for ST != -1");
  static void Print(Tp const&) { std::clog << "Value Not Streamable!\n"; }
};

template <class Tp>
struct SelectStream<Tp, 1> {
  static void Print(Tp const& val) { std::cerr << val; }
};

template <class Tp>
struct SelectStream<Tp, 2> {
  static void Print(Tp const& val) { std::wcerr << val; }
};

struct AssertData {
  AssertData(const char* xcheck, const char* xfile, const char* xfunc,
             unsigned long xline, bool xpassed = true)
      : passed(xpassed), check(xcheck), file(xfile), func(xfunc), line(xline),
        msg() {}

  AssertData& SetFailed(std::string xmsg = std::string()) {
    msg = xmsg;
    passed = false;
    return *this;
  }

  void PrintFailed() const {
    std::fprintf(stderr, "%s:%lu %s: Assertion '%s' failed.\n", file, line,
                 func, check);
    if (!msg.empty())
      std::fprintf(stderr, "%s\n", msg.data());
  }

  bool passed;
  const char* check;
  const char* file;
  const char* func;
  unsigned long line;
  std::string msg;
};

// AssertHandler is the class constructed by failing CHECK macros. AssertHandler
// will log information about the failures and abort when it is destructed.
class AssertHandler {
public:
  AssertHandler(AssertData const& Data)
      : passed(Data.passed) {
    if (!passed)
      Data.PrintFailed();
  }

  ~AssertHandler() TEST_NOEXCEPT_FALSE {
    if (!passed) {
      error_log << std::endl;
      std::abort();
    }
  }

  class LogType {
    friend class AssertHandler;

    template <class Tp>
    friend LogType& operator<<(LogType& log, Tp const& value) {
      if (!log.is_disabled) {
        SelectStream<Tp>::Print(value);
      }
      return log;
    }

    friend LogType& operator<<(LogType& log, EndLType* m) {
      if (!log.is_disabled) {
        SelectStream<EndLType*>::Print(m);
      }
      return log;
    }

  private:
    LogType(bool disable) : is_disabled(disable) {}
    bool is_disabled;

    LogType(LogType const&);
    LogType& operator=(LogType const&);
  };

  LogType& GetLog() {
    if (passed)
      return null_log;
    return error_log;
  }

private:
  static LogType null_log;
  static LogType error_log;

  AssertHandler& operator=(const AssertHandler&) = delete;
  AssertHandler(const AssertHandler&) = delete;
  AssertHandler() = delete;

private:
  bool passed;
};

AssertHandler::LogType AssertHandler::null_log(true);
AssertHandler::LogType AssertHandler::error_log(false);

template <class It1>
std::string PrintRange(const char* Name, It1 F, It1 E) {
  std::stringstream ss;
  ss << "  " << Name << " = [";
  while (F != E) {
    ss << *F;
    ++F;
    if (F != E)
      ss << ", ";
  }
  ss << "]\n";
  return ss.str();
}

template <class Tp, class Up>
std::string PrintMismatch(Tp const& LHS, Up const& RHS, int Elem) {
  std::stringstream ss;
  ss << "  Element " << Elem << " mismatched: `" << LHS << "` != `" << RHS
     << "`!\n";
  return ss.str();
};

struct EqualToComp {
  template <class Tp, class Up>
  bool operator()(Tp const& LHS, Up const& RHS) const {
    return LHS == RHS;
  }
};

template <class It1, class It2, class Comp>
AssertData CheckCollectionsEqual(It1 F1, It1 E1, It2 F2, It2 E2,
                                 AssertData Data, Comp C = EqualToComp()) {
  const It1 F1Orig = F1;
  const It2 F2Orig = F2;
  bool Failed = false;
  std::string ErrorMsg;
  int Idx = 0;
  while (F1 != E1 && F2 != E2) {
    if (!(C(*F1, *F2))) {
      ErrorMsg += PrintMismatch(*F1, *F2, Idx);
      Failed = true;
      break;
    }
    ++Idx;
    ++F1;
    ++F2;
  }
  if (!Failed && (F1 != E1 || F2 != E2)) {
    ErrorMsg += "  Ranges have different sizes!\n";
    Failed = true;
  }
  if (Failed) {
    ErrorMsg += PrintRange("LHS", F1Orig, E1);
    ErrorMsg += PrintRange("RHS", F2Orig, E2);
    Data.SetFailed(ErrorMsg);
  }
  return Data;
}
} // namespace verbose_assert

#ifdef __GNUC__
#define ASSERT_FN_NAME() __PRETTY_FUNCTION__
#else
#define ASSERT_FN_NAME() __func__
#endif

#define DISPLAY(...) "    " #__VA_ARGS__ " = " << (__VA_ARGS__) << "\n"

#define ASSERT(...)                                                            \
  ::verbose_assert::AssertHandler(::verbose_assert::AssertData(                \
    #__VA_ARGS__, __FILE__, ASSERT_FN_NAME(), __LINE__,(__VA_ARGS__))).GetLog()

#define ASSERT_EQ(LHS, RHS) \
  ASSERT(LHS == RHS) << DISPLAY(LHS) << DISPLAY(RHS)
#define ASSERT_NEQ(LHS, RHS) \
  ASSERT(LHS != RHS) << DISPLAY(LHS) << DISPLAY(RHS)
#define ASSERT_PRED(PRED, LHS, RHS) \
  ASSERT(PRED(LHS, RHS)) << DISPLAY(LHS) << DISPLAY(RHS)

#define ASSERT_COLLECTION_EQ_COMP(F1, E1, F2, E2, Comp)                        \
  (::verbose_assert::AssertHandler(                                            \
       ::verbose_assert::CheckCollectionsEqual(                                \
           F1, E1, F2, E2,                                                     \
           ::verbose_assert::AssertData("CheckCollectionsEqual(" #F1 ", " #E1  \
                                        ", " #F2 ", " #E2 ")",                 \
                                        __FILE__, ASSERT_FN_NAME(), __LINE__), \
           Comp))                                                              \
       .GetLog())

#define ASSERT_COLLECTION_EQ(F1, E1, F2, E2)                                   \
  ASSERT_COLLECTION_EQ_COMP(F1, E1, F2, E2, ::verbose_assert::EqualToComp())

#endif
