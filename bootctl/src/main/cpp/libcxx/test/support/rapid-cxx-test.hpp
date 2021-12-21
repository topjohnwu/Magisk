#ifndef RAPID_CXX_TEST_HPP
#define RAPID_CXX_TEST_HPP

# include <cstddef>
# include <cstdlib>
# include <cstdio>
# include <cstring>
# include <cassert>

#include "test_macros.h"

#if !defined(RAPID_CXX_TEST_NO_SYSTEM_HEADER) || !defined(__GNUC__)
#pragma GCC system_header
#endif

# define RAPID_CXX_TEST_PP_CAT(x, y) RAPID_CXX_TEST_PP_CAT_2(x, y)
# define RAPID_CXX_TEST_PP_CAT_2(x, y) x##y

# define RAPID_CXX_TEST_PP_STR(...) RAPID_CXX_TEST_PP_STR_2(__VA_ARGS__)
# define RAPID_CXX_TEST_PP_STR_2(...) #__VA_ARGS__

# if defined(__GNUC__)
#   define TEST_FUNC_NAME() __PRETTY_FUNCTION__
#   define RAPID_CXX_TEST_UNUSED __attribute__((unused))
# else
#   define TEST_FUNC_NAME() __func__
#   define RAPID_CXX_TEST_UNUSED
# endif

////////////////////////////////////////////////////////////////////////////////
//                          TEST_SUITE
////////////////////////////////////////////////////////////////////////////////
# define TEST_SUITE(Name)                                           \
namespace Name                                                      \
{                                                                   \
    inline ::rapid_cxx_test::test_suite & get_test_suite()          \
    {                                                               \
        static ::rapid_cxx_test::test_suite m_suite(#Name);         \
        return m_suite;                                             \
    }                                                               \
                                                                    \
    inline int unit_test_main(int, char**)                          \
    {                                                               \
        ::rapid_cxx_test::test_runner runner(get_test_suite());     \
        return runner.run();                                        \
    }                                                               \
}                                                                   \
int main(int argc, char **argv)                                     \
{                                                                   \
    return Name::unit_test_main(argc, argv);                        \
}                                                                   \
namespace Name                                                      \
{ /* namespace closed in TEST_SUITE_END */
#

////////////////////////////////////////////////////////////////////////////////
//                         TEST_SUITE_END
////////////////////////////////////////////////////////////////////////////////
# define TEST_SUITE_END()                                       \
} /* namespace opened in TEST_SUITE(...) */
#

////////////////////////////////////////////////////////////////////////////////
//                          TEST_CASE
////////////////////////////////////////////////////////////////////////////////

# if !defined(__clang__)
#
# define TEST_CASE(Name)                                                                                \
    void Name();                                                                                        \
    static void RAPID_CXX_TEST_PP_CAT(Name, _invoker)()                                                 \
    {                                                                                                   \
        Name();                                                                                         \
    }                                                                                                   \
    static ::rapid_cxx_test::registrar                                                                  \
    RAPID_CXX_TEST_PP_CAT(rapid_cxx_test_registrar_, Name)(                                         \
        get_test_suite()                                                                                \
      , ::rapid_cxx_test::test_case(__FILE__, #Name, __LINE__, & RAPID_CXX_TEST_PP_CAT(Name, _invoker)) \
      );                                                                                                \
    void Name()
#
# else /* __clang__ */
#
# define TEST_CASE(Name)                                                                                \
    void Name();                                                                                        \
    static void RAPID_CXX_TEST_PP_CAT(Name, _invoker)()                                                 \
    {                                                                                                   \
        Name();                                                                                         \
    }                                                                                                   \
    _Pragma("clang diagnostic push")                                                                    \
    _Pragma("clang diagnostic ignored \"-Wglobal-constructors\"")                                       \
    static ::rapid_cxx_test::registrar                                                                  \
    RAPID_CXX_TEST_PP_CAT(rapid_cxx_test_registrar_, Name)(                                         \
        get_test_suite()                                                                                \
      , ::rapid_cxx_test::test_case(__FILE__, #Name, __LINE__, & RAPID_CXX_TEST_PP_CAT(Name, _invoker)) \
      );                                                                                                \
    _Pragma("clang diagnostic pop")                                                                     \
    void Name()
#
# endif /* !defined(__clang__) */


# define TEST_SET_CHECKPOINT() ::rapid_cxx_test::set_checkpoint(__FILE__, TEST_FUNC_NAME(), __LINE__)

#define RAPID_CXX_TEST_OUTCOME()

////////////////////////////////////////////////////////////////////////////////
//                              TEST_UNSUPPORTED
////////////////////////////////////////////////////////////////////////////////
# define TEST_UNSUPPORTED()                                                                 \
    do {                                                                                    \
        TEST_SET_CHECKPOINT();                                                              \
        ::rapid_cxx_test::test_outcome m_f(                                                 \
          ::rapid_cxx_test::failure_type::unsupported, __FILE__, TEST_FUNC_NAME(), __LINE__ \
          , "", ""                                                                          \
        );                                                                                  \
        ::rapid_cxx_test::get_reporter().report(m_f);                                       \
        return;                                                                             \
    } while (false)
#


////////////////////////////////////////////////////////////////////////////////
//                            BASIC ASSERTIONS
////////////////////////////////////////////////////////////////////////////////
# define TEST_WARN(...)                                                                \
    do {                                                                               \
        TEST_SET_CHECKPOINT();                                                         \
        ::rapid_cxx_test::test_outcome m_f(                                            \
            ::rapid_cxx_test::failure_type::none, __FILE__, TEST_FUNC_NAME(), __LINE__ \
            , "TEST_WARN(" #__VA_ARGS__ ")", ""                                        \
            );                                                                         \
        if (not (__VA_ARGS__)) {                                                       \
            m_f.type = ::rapid_cxx_test::failure_type::warn;                           \
        }                                                                              \
        ::rapid_cxx_test::get_reporter().report(m_f);                                  \
    } while (false)
#

# define TEST_CHECK(...)                                                               \
    do {                                                                               \
        TEST_SET_CHECKPOINT();                                                         \
        ::rapid_cxx_test::test_outcome m_f(                                            \
            ::rapid_cxx_test::failure_type::none, __FILE__, TEST_FUNC_NAME(), __LINE__ \
            , "TEST_CHECK(" #__VA_ARGS__ ")", ""                                       \
            );                                                                         \
        if (not (__VA_ARGS__)) {                                                       \
            m_f.type = ::rapid_cxx_test::failure_type::check;                          \
        }                                                                              \
        ::rapid_cxx_test::get_reporter().report(m_f);                                  \
    } while (false)
#

# define TEST_REQUIRE(...)                                                             \
    do {                                                                               \
        TEST_SET_CHECKPOINT();                                                         \
        ::rapid_cxx_test::test_outcome m_f(                                            \
            ::rapid_cxx_test::failure_type::none, __FILE__, TEST_FUNC_NAME(), __LINE__ \
            , "TEST_REQUIRE(" #__VA_ARGS__ ")", ""                                     \
            );                                                                         \
        if (not (__VA_ARGS__)) {                                                       \
            m_f.type = ::rapid_cxx_test::failure_type::require;                        \
        }                                                                              \
        ::rapid_cxx_test::get_reporter().report(m_f);                                  \
        if (m_f.type != ::rapid_cxx_test::failure_type::none) {                        \
            return;                                                                    \
        }                                                                              \
    } while (false)
#

# define TEST_ASSERT(...)                                                              \
    do {                                                                               \
        TEST_SET_CHECKPOINT();                                                         \
        ::rapid_cxx_test::test_outcome m_f(                                            \
            ::rapid_cxx_test::failure_type::none, __FILE__, TEST_FUNC_NAME(), __LINE__ \
            , "TEST_ASSERT(" #__VA_ARGS__ ")", ""                                      \
            );                                                                         \
        if (not (__VA_ARGS__)) {                                                       \
            m_f.type = ::rapid_cxx_test::failure_type::assert;                         \
        }                                                                              \
        ::rapid_cxx_test::get_reporter().report(m_f);                                  \
        if (m_f.type != ::rapid_cxx_test::failure_type::none) {                        \
            std::abort();                                                              \
        }                                                                              \
    } while (false)
#

////////////////////////////////////////////////////////////////////////////////
//                    TEST_CHECK_NO_THROW / TEST_CHECK_THROW
////////////////////////////////////////////////////////////////////////////////
#ifndef TEST_HAS_NO_EXCEPTIONS

# define TEST_CHECK_NO_THROW(...)                                                      \
    do {                                                                               \
        TEST_SET_CHECKPOINT();                                                         \
        ::rapid_cxx_test::test_outcome m_f(                                            \
            ::rapid_cxx_test::failure_type::none, __FILE__, TEST_FUNC_NAME(), __LINE__ \
            , "TEST_CHECK_NO_THROW(" #__VA_ARGS__ ")", ""                              \
            );                                                                         \
        try {                                                                          \
            (static_cast<void>(__VA_ARGS__));                                          \
        } catch (...) {                                                                \
            m_f.type = ::rapid_cxx_test::failure_type::check;                          \
        }                                                                              \
        ::rapid_cxx_test::get_reporter().report(m_f);                                  \
    } while (false)
#

# define TEST_CHECK_THROW(Except, ...)                                                 \
    do {                                                                               \
        TEST_SET_CHECKPOINT();                                                         \
        ::rapid_cxx_test::test_outcome m_f(                                            \
            ::rapid_cxx_test::failure_type::none, __FILE__, TEST_FUNC_NAME(), __LINE__ \
            , "TEST_CHECK_THROW(" #Except "," #__VA_ARGS__ ")", ""                     \
            );                                                                         \
        try {                                                                          \
            (static_cast<void>(__VA_ARGS__));                                          \
            m_f.type = ::rapid_cxx_test::failure_type::check;                          \
        } catch (Except const &) {}                                                    \
        ::rapid_cxx_test::get_reporter().report(m_f);                                  \
    } while (false)
#

#define TEST_CHECK_THROW_RESULT(Except, Checker, ...)                          \
  do {                                                                         \
    TEST_SET_CHECKPOINT();                                                     \
    ::rapid_cxx_test::test_outcome m_f(::rapid_cxx_test::failure_type::none,   \
                                       __FILE__, TEST_FUNC_NAME(), __LINE__,   \
                                       "TEST_CHECK_THROW_RESULT(" #Except      \
                                       "," #Checker "," #__VA_ARGS__ ")",      \
                                       "");                                    \
    try {                                                                      \
      (static_cast<void>(__VA_ARGS__));                                        \
      m_f.type = ::rapid_cxx_test::failure_type::check;                        \
    } catch (Except const& Caught) {                                           \
      Checker(Caught);                                                         \
    }                                                                          \
    ::rapid_cxx_test::get_reporter().report(m_f);                              \
  } while (false)
#

#else // TEST_HAS_NO_EXCEPTIONS

# define TEST_CHECK_NO_THROW(...)                                                      \
    do {                                                                               \
        TEST_SET_CHECKPOINT();                                                         \
        ::rapid_cxx_test::test_outcome m_f(                                            \
            ::rapid_cxx_test::failure_type::none, __FILE__, TEST_FUNC_NAME(), __LINE__ \
            , "TEST_CHECK_NO_THROW(" #__VA_ARGS__ ")", ""                              \
            );                                                                         \
        (static_cast<void>(__VA_ARGS__));                                              \
        ::rapid_cxx_test::get_reporter().report(m_f);                                  \
    } while (false)
#

#define TEST_CHECK_THROW(Except, ...) ((void)0)
#define TEST_CHECK_THROW_RESULT(Except, Checker, ...) ((void)0)

#endif // TEST_HAS_NO_EXCEPTIONS


////////////////////////////////////////////////////////////////////////////////
//                    TEST_REQUIRE_NO_THROW / TEST_REQUIRE_THROWs
////////////////////////////////////////////////////////////////////////////////
#ifndef TEST_HAS_NO_EXCEPTIONS

# define TEST_REQUIRE_NO_THROW(...)                                                    \
    do {                                                                               \
        TEST_SET_CHECKPOINT();                                                         \
        ::rapid_cxx_test::test_outcome m_f(                                            \
            ::rapid_cxx_test::failure_type::none, __FILE__, TEST_FUNC_NAME(), __LINE__ \
            , "TEST_REQUIRE_NO_THROW(" #__VA_ARGS__ ")", ""                            \
            );                                                                         \
        try {                                                                          \
            (static_cast<void>(__VA_ARGS__));                                          \
        } catch (...) {                                                                \
            m_f.type = ::rapid_cxx_test::failure_type::require;                        \
        }                                                                              \
        ::rapid_cxx_test::get_reporter().report(m_f);                                  \
        if (m_f.type != ::rapid_cxx_test::failure_type::none) {                        \
            return;                                                                    \
        }                                                                              \
    } while (false)
#

# define TEST_REQUIRE_THROW(Except, ...)                                               \
    do {                                                                               \
        TEST_SET_CHECKPOINT();                                                         \
        ::rapid_cxx_test::test_outcome m_f(                                            \
            ::rapid_cxx_test::failure_type::none, __FILE__, TEST_FUNC_NAME(), __LINE__ \
            , "TEST_REQUIRE_THROW(" #Except "," #__VA_ARGS__ ")", ""                   \
            );                                                                         \
        try {                                                                          \
            (static_cast<void>(__VA_ARGS__));                                          \
            m_f.type = ::rapid_cxx_test::failure_type::require;                        \
        } catch (Except const &) {}                                                    \
        ::rapid_cxx_test::get_reporter().report(m_f);                                  \
        if (m_f.type != ::rapid_cxx_test::failure_type::none) {                        \
            return;                                                                    \
        }                                                                              \
    } while (false)
#

#else // TEST_HAS_NO_EXCEPTIONS

# define TEST_REQUIRE_NO_THROW(...)                                                    \
    do {                                                                               \
        TEST_SET_CHECKPOINT();                                                         \
        ::rapid_cxx_test::test_outcome m_f(                                            \
            ::rapid_cxx_test::failure_type::none, __FILE__, TEST_FUNC_NAME(), __LINE__ \
            , "TEST_REQUIRE_NO_THROW(" #__VA_ARGS__ ")", ""                            \
            );                                                                         \
        (static_cast<void>(__VA_ARGS__));                                              \
        ::rapid_cxx_test::get_reporter().report(m_f);                                  \
    } while (false)
#

#define TEST_REQUIRE_THROW(Except, ...) ((void)0)

#endif // TEST_HAS_NO_EXCEPTIONS

////////////////////////////////////////////////////////////////////////////////
//                    TEST_ASSERT_NO_THROW / TEST_ASSERT_THROW
////////////////////////////////////////////////////////////////////////////////
#ifndef TEST_HAS_NO_EXCEPTIONS

# define TEST_ASSERT_NO_THROW(...)                                                     \
    do {                                                                               \
        TEST_SET_CHECKPOINT();                                                         \
        ::rapid_cxx_test::test_outcome m_f(                                            \
            ::rapid_cxx_test::failure_type::none, __FILE__, TEST_FUNC_NAME(), __LINE__ \
            , "TEST_ASSERT_NO_THROW(" #__VA_ARGS__ ")", ""                             \
            );                                                                         \
        try {                                                                          \
            (static_cast<void>(__VA_ARGS__));                                          \
        } catch (...) {                                                                \
            m_f.type = ::rapid_cxx_test::failure_type::assert;                         \
        }                                                                              \
        ::rapid_cxx_test::get_reporter().report(m_f);                                  \
        if (m_f.type != ::rapid_cxx_test::failure_type::none) {                        \
            std::abort();                                                              \
        }                                                                              \
    } while (false)
#

# define TEST_ASSERT_THROW(Except, ...)                                                \
    do {                                                                               \
        TEST_SET_CHECKPOINT();                                                         \
        ::rapid_cxx_test::test_outcome m_f(                                            \
            ::rapid_cxx_test::failure_type::none, __FILE__, TEST_FUNC_NAME(), __LINE__ \
            , "TEST_ASSERT_THROW(" #Except "," #__VA_ARGS__ ")", ""                    \
            );                                                                         \
        try {                                                                          \
            (static_cast<void>(__VA_ARGS__));                                          \
            m_f.type = ::rapid_cxx_test::failure_type::assert;                         \
        } catch (Except const &) {}                                                    \
        ::rapid_cxx_test::get_reporter().report(m_f);                                  \
        if (m_f.type != ::rapid_cxx_test::failure_type::none) {                        \
            std::abort();                                                              \
        }                                                                              \
    } while (false)
#

#else // TEST_HAS_NO_EXCEPTIONS

# define TEST_ASSERT_NO_THROW(...)                                                     \
    do {                                                                               \
        TEST_SET_CHECKPOINT();                                                         \
        ::rapid_cxx_test::test_outcome m_f(                                            \
            ::rapid_cxx_test::failure_type::none, __FILE__, TEST_FUNC_NAME(), __LINE__ \
            , "TEST_ASSERT_NO_THROW(" #__VA_ARGS__ ")", ""                             \
            );                                                                         \
        (static_cast<void>(__VA_ARGS__));                                              \
        ::rapid_cxx_test::get_reporter().report(m_f);                                  \
    } while (false)
#

#define TEST_ASSERT_THROW(Except, ...) ((void)0)

#endif // TEST_HAS_NO_EXCEPTIONS

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

# define TEST_WARN_EQUAL_COLLECTIONS(...)                                              \
    do {                                                                               \
        TEST_SET_CHECKPOINT();                                                         \
        ::rapid_cxx_test::test_outcome m_f(                                            \
          ::rapid_cxx_test::failure_type::none, __FILE__, TEST_FUNC_NAME(), __LINE__   \
          , "TEST_WARN_EQUAL_COLLECTIONS(" #__VA_ARGS__ ")", ""                        \
        );                                                                             \
        if (not ::rapid_cxx_test::detail::check_equal_collections_impl(__VA_ARGS__)) { \
            m_f.type = ::rapid_cxx_test::failure_type::warn;                           \
        }                                                                              \
        ::rapid_cxx_test::get_reporter().report(m_f);                                  \
    } while (false)
#

# define TEST_CHECK_EQUAL_COLLECTIONS(...)                                             \
    do {                                                                               \
        TEST_SET_CHECKPOINT();                                                         \
        ::rapid_cxx_test::test_outcome m_f(                                            \
          ::rapid_cxx_test::failure_type::none, __FILE__, TEST_FUNC_NAME(), __LINE__   \
          , "TEST_CHECK_EQUAL_COLLECTIONS(" #__VA_ARGS__ ")", ""                       \
        );                                                                             \
        if (not ::rapid_cxx_test::detail::check_equal_collections_impl(__VA_ARGS__)) { \
            m_f.type = ::rapid_cxx_test::failure_type::check;                          \
        }                                                                              \
        ::rapid_cxx_test::get_reporter().report(m_f);                                  \
    } while (false)
#

# define TEST_REQUIRE_EQUAL_COLLECTIONS(...)                                           \
    do {                                                                               \
        TEST_SET_CHECKPOINT();                                                         \
        ::rapid_cxx_test::test_outcome m_f(                                            \
          ::rapid_cxx_test::failure_type::none, __FILE__, TEST_FUNC_NAME(), __LINE__   \
          , "TEST_REQUIRE_EQUAL_COLLECTIONS(" #__VA_ARGS__ ")", ""                     \
        );                                                                             \
        if (not ::rapid_cxx_test::detail::check_equal_collections_impl(__VA_ARGS__)) { \
            m_f.type = ::rapid_cxx_test::failure_type::require;                        \
        }                                                                              \
        ::rapid_cxx_test::get_reporter().report(m_f);                                  \
        if (m_f.type != ::rapid_cxx_test::failure_type::none) {                        \
            return;                                                                    \
        }                                                                              \
    } while (false)
#

# define TEST_ASSERT_EQUAL_COLLECTIONS(...)                                            \
    do {                                                                               \
        TEST_SET_CHECKPOINT();                                                         \
        ::rapid_cxx_test::test_outcome m_f(                                            \
          ::rapid_cxx_test::failure_type::none, __FILE__, TEST_FUNC_NAME(), __LINE__   \
          , "TEST_ASSERT_EQUAL_COLLECTIONS(" #__VA_ARGS__ ")", ""                      \
        );                                                                             \
        if (not ::rapid_cxx_test::detail::check_equal_collections_impl(__VA_ARGS__)) { \
            m_f.type = ::rapid_cxx_test::failure_type::assert;                         \
        }                                                                              \
        ::rapid_cxx_test::get_reporter().report(m_f);                                  \
        if (m_f.type != ::rapid_cxx_test::failure_type::none) {                        \
          ::std::abort();                                                              \
        }                                                                              \
    } while (false)
#

namespace rapid_cxx_test
{
    typedef void (*invoker_t)();

    ////////////////////////////////////////////////////////////////////////////
    struct test_case
    {
        test_case()
            : file(""), func(""), line(0), invoke(NULL)
        {}

        test_case(const char* file1, const char* func1, std::size_t line1,
                  invoker_t invoke1)
            : file(file1), func(func1), line(line1), invoke(invoke1)
        {}

        const char *file;
        const char *func;
        std::size_t line;
        invoker_t invoke;
    };

    ////////////////////////////////////////////////////////////////////////////
    struct failure_type
    {
        enum enum_type {
            none,
            unsupported,
            warn,
            check,
            require,
            assert,
            uncaught_exception
        };
    };

    typedef failure_type::enum_type failure_type_t;

    ////////////////////////////////////////////////////////////////////////////
    struct test_outcome
    {
        test_outcome()
            : type(failure_type::none),
              file(""), func(""), line(0),
              expression(""), message("")
        {}

        test_outcome(failure_type_t type1, const char* file1, const char* func1,
                     std::size_t line1, const char* expression1,
                     const char* message1)
            : type(type1), file(file1), func(func1), line(line1),
              expression(expression1), message(message1)
        {
            trim_func_string();
        }

        failure_type_t type;
        const char *file;
        const char *func;
        std::size_t line;
        const char *expression;
        const char *message;

    private:
        void trim_file_string() {
            const char* f_start  = file;
            const char* prev_start = f_start;
            const char* last_start = f_start;
            char last;
            while ((last = *f_start) != '\0') {
                ++f_start;
                if (last == '/' && *f_start) {
                    prev_start = last_start;
                    last_start = f_start;
                }
            }
            file = prev_start;
        }
      void trim_func_string() {
          const char* void_loc = ::strstr(func, "void ");
          if (void_loc == func) {
              func += strlen("void ");
          }
          const char* namespace_loc = ::strstr(func, "::");
          if (namespace_loc) {
              func = namespace_loc + 2;
          }
      }
    };

    ////////////////////////////////////////////////////////////////////////////
    struct checkpoint
    {
        const char *file;
        const char *func;
        std::size_t line;
    };

    namespace detail
    {
        inline checkpoint & global_checkpoint()
        {
            static checkpoint cp = {"", "", 0};
            return cp;
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    inline void set_checkpoint(const char* file, const char* func, std::size_t line)
    {
        checkpoint& cp = detail::global_checkpoint();
        cp.file = file;
        cp.func = func;
        cp.line = line;
    }

    ////////////////////////////////////////////////////////////////////////////
    inline checkpoint const & get_checkpoint()
    {
        return detail::global_checkpoint();
    }

    ////////////////////////////////////////////////////////////////////////////
    class test_suite
    {
    public:
        typedef test_case const* iterator;
        typedef iterator const_iterator;

    public:
        test_suite(const char *xname)
          : m_name(xname), m_tests(), m_size(0)
        {
            assert(xname);
        }

    public:
        const char *name() const { return m_name; }

        std::size_t size() const { return m_size; }

        test_case const & operator[](std::size_t i) const
        {
            assert(i < m_size);
            return m_tests[i];
        }

        const_iterator begin() const
        { return m_tests; }

        const_iterator end() const
        {
            return m_tests + m_size;
        }

    public:
        std::size_t register_test(test_case tc)
        {
            static std::size_t test_case_max = sizeof(m_tests) / sizeof(test_case);
            assert(m_size < test_case_max);
            m_tests[m_size] = tc;
            return m_size++;
        }

    private:
        test_suite(test_suite const &);
        test_suite & operator=(test_suite const &);

    private:
        const char* m_name;
        // Since fast compile times in a priority, we use simple containers
        // with hard limits.
        test_case m_tests[256];
        std::size_t m_size;
    };

    ////////////////////////////////////////////////////////////////////////////
    class registrar
    {
    public:
        registrar(test_suite & st, test_case tc)
        {
            st.register_test(tc);
        }
    };

    ////////////////////////////////////////////////////////////////////////////
    class test_reporter
    {
    public:
        test_reporter()
            : m_testcases(0), m_testcase_failures(0), m_unsupported(0),
              m_assertions(0), m_warning_failures(0), m_check_failures(0),
              m_require_failures(0), m_uncaught_exceptions(0), m_failure()
        {
        }

        void test_case_begin()
        {
            ++m_testcases;
            clear_failure();
        }

        void test_case_end()
        {
            if (m_failure.type != failure_type::none
                && m_failure.type !=  failure_type::unsupported) {
                ++m_testcase_failures;
            }
        }

# if defined(__GNUC__)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wswitch-default"
# endif
        // Each assertion and failure is reported through this function.
        void report(test_outcome o)
        {
            ++m_assertions;
            switch (o.type)
            {
            case failure_type::none:
                break;
            case failure_type::unsupported:
                ++m_unsupported;
                m_failure = o;
                break;
            case failure_type::warn:
                ++m_warning_failures;
                report_error(o);
                break;
            case failure_type::check:
                ++m_check_failures;
                report_error(o);
                m_failure = o;
                break;
            case failure_type::require:
                ++m_require_failures;
                report_error(o);
                m_failure = o;
                break;
            case failure_type::assert:
                report_error(o);
                break;
            case failure_type::uncaught_exception:
                ++m_uncaught_exceptions;
                std::fprintf(stderr
                    , "Test case FAILED with uncaught exception:\n"
                      "    last checkpoint near %s::%lu %s\n\n"
                    , o.file, o.line, o.func
                    );
                m_failure = o;
                break;
            }
        }
# if defined(__GNUC__)
#   pragma GCC diagnostic pop
# endif

        test_outcome current_failure() const
        {
            return m_failure;
        }

        void clear_failure()
        {
            m_failure.type = failure_type::none;
            m_failure.file = "";
            m_failure.func = "";
            m_failure.line = 0;
            m_failure.expression = "";
            m_failure.message = "";
        }

        std::size_t test_case_count() const
        { return m_testcases; }

        std::size_t test_case_failure_count() const
        { return m_testcase_failures; }

        std::size_t unsupported_count() const
        { return m_unsupported; }

        std::size_t assertion_count() const
        { return m_assertions; }

        std::size_t warning_failure_count() const
        { return m_warning_failures; }

        std::size_t check_failure_count() const
        { return m_check_failures; }

        std::size_t require_failure_count() const
        { return m_require_failures; }

        std::size_t failure_count() const
        { return m_check_failures + m_require_failures + m_uncaught_exceptions; }

        // Print a summary of what was run and the outcome.
        void print_summary(const char* suitename) const
        {
            FILE* out = failure_count() ? stderr : stdout;
            std::size_t testcases_run = m_testcases - m_unsupported;
            std::fprintf(out, "Summary for testsuite %s:\n", suitename);
            std::fprintf(out, "    %lu of %lu test cases passed.\n", testcases_run - m_testcase_failures, testcases_run);
            std::fprintf(out, "    %lu of %lu assertions passed.\n", m_assertions - (m_warning_failures + m_check_failures + m_require_failures), m_assertions);
            std::fprintf(out, "    %lu unsupported test case%s.\n", m_unsupported, (m_unsupported != 1 ? "s" : ""));
        }

    private:
        test_reporter(test_reporter const &);
        test_reporter const & operator=(test_reporter const &);

        void report_error(test_outcome o) const
        {
            std::fprintf(stderr, "In %s:%lu Assertion %s failed.\n    in file: %s\n    %s\n"
                , o.func, o.line, o.expression, o.file,  o.message ? o.message : ""
              );
        }

    private:
        // counts of testcases, failed testcases, and unsupported testcases.
        std::size_t m_testcases;
        std::size_t m_testcase_failures;
        std::size_t m_unsupported;

        // counts of assertions and assertion failures.
        std::size_t m_assertions;
        std::size_t m_warning_failures;
        std::size_t m_check_failures;
        std::size_t m_require_failures;
        std::size_t m_uncaught_exceptions;

        // The last failure. This is cleared between testcases.
        test_outcome m_failure;
    };

    ////////////////////////////////////////////////////////////////////////////
    inline test_reporter & get_reporter()
    {
        static test_reporter o;
        return o;
    }

    ////////////////////////////////////////////////////////////////////////////
    class test_runner
    {
    public:
        test_runner(test_suite & ts)
          : m_ts(ts)
        {}

    public:
        int run()
        {
            // for each testcase
            for (test_suite::const_iterator b = m_ts.begin(), e = m_ts.end();
                 b != e; ++b)
            {
                test_case const& tc = *b;
                set_checkpoint(tc.file, tc.func, tc.line);
                get_reporter().test_case_begin();
#ifndef TEST_HAS_NO_EXCEPTIONS
                try {
#endif
                    tc.invoke();
#ifndef TEST_HAS_NO_EXCEPTIONS
                } catch (...) {
                    test_outcome o;
                    o.type = failure_type::uncaught_exception;
                    o.file = get_checkpoint().file;
                    o.func = get_checkpoint().func;
                    o.line = get_checkpoint().line;
                    o.expression = "";
                    o.message = "";
                    get_reporter().report(o);
                }
#endif
                get_reporter().test_case_end();
            }
            auto exit_code = get_reporter().failure_count() ? EXIT_FAILURE : EXIT_SUCCESS;
            if (exit_code == EXIT_FAILURE || get_reporter().unsupported_count())
              get_reporter().print_summary(m_ts.name());
            return exit_code;
        }

    private:
        test_runner(test_runner const &);
        test_runner operator=(test_runner const &);

        test_suite & m_ts;
    };

    namespace detail
    {
        template <class Iter1, class Iter2>
        bool check_equal_collections_impl(
            Iter1 start1, Iter1 const end1
          , Iter2 start2, Iter2 const end2
          )
        {
            while (start1 != end1 && start2 != end2) {
                if (*start1 != *start2) {
                    return false;
                }
                ++start1; ++start2;
            }
            return (start1 == end1 && start2 == end2);
        }
    }                                                       // namespace detail

}                                                    // namespace rapid_cxx_test


# if defined(__GNUC__)
#   pragma GCC diagnostic pop
# endif

#endif /* RAPID_CXX_TEST_HPP */
