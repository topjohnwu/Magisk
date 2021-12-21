# Enable the tests

find_package(Threads REQUIRED)
include(CheckCXXCompilerFlag)

# NOTE: Some tests use `<cassert>` to perform the test. Therefore we must
# strip -DNDEBUG from the default CMake flags in DEBUG mode.
string(TOUPPER "${CMAKE_BUILD_TYPE}" uppercase_CMAKE_BUILD_TYPE)
if( NOT uppercase_CMAKE_BUILD_TYPE STREQUAL "DEBUG" )
  add_definitions( -UNDEBUG )
  add_definitions(-DTEST_BENCHMARK_LIBRARY_HAS_NO_ASSERTIONS)
  # Also remove /D NDEBUG to avoid MSVC warnings about conflicting defines.
  foreach (flags_var_to_scrub
      CMAKE_CXX_FLAGS_RELEASE
      CMAKE_CXX_FLAGS_RELWITHDEBINFO
      CMAKE_CXX_FLAGS_MINSIZEREL
      CMAKE_C_FLAGS_RELEASE
      CMAKE_C_FLAGS_RELWITHDEBINFO
      CMAKE_C_FLAGS_MINSIZEREL)
    string (REGEX REPLACE "(^| )[/-]D *NDEBUG($| )" " "
      "${flags_var_to_scrub}" "${${flags_var_to_scrub}}")
  endforeach()
endif()

check_cxx_compiler_flag(-O3 BENCHMARK_HAS_O3_FLAG)
set(BENCHMARK_O3_FLAG "")
if (BENCHMARK_HAS_O3_FLAG)
  set(BENCHMARK_O3_FLAG "-O3")
endif()

# NOTE: These flags must be added after find_package(Threads REQUIRED) otherwise
# they will break the configuration check.
if (DEFINED BENCHMARK_CXX_LINKER_FLAGS)
  list(APPEND CMAKE_EXE_LINKER_FLAGS ${BENCHMARK_CXX_LINKER_FLAGS})
endif()

add_library(output_test_helper STATIC output_test_helper.cc output_test.h)

macro(compile_benchmark_test name)
  add_executable(${name} "${name}.cc")
  target_link_libraries(${name} benchmark ${CMAKE_THREAD_LIBS_INIT})
endmacro(compile_benchmark_test)

macro(compile_benchmark_test_with_main name)
  add_executable(${name} "${name}.cc")
  target_link_libraries(${name} benchmark_main)
endmacro(compile_benchmark_test_with_main)

macro(compile_output_test name)
  add_executable(${name} "${name}.cc" output_test.h)
  target_link_libraries(${name} output_test_helper benchmark
          ${BENCHMARK_CXX_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
endmacro(compile_output_test)

# Demonstration executable
compile_benchmark_test(benchmark_test)
add_test(benchmark benchmark_test --benchmark_min_time=0.01)

compile_benchmark_test(filter_test)
macro(add_filter_test name filter expect)
  add_test(${name} filter_test --benchmark_min_time=0.01 --benchmark_filter=${filter} ${expect})
  add_test(${name}_list_only filter_test --benchmark_list_tests --benchmark_filter=${filter} ${expect})
endmacro(add_filter_test)

add_filter_test(filter_simple "Foo" 3)
add_filter_test(filter_simple_negative "-Foo" 2)
add_filter_test(filter_suffix "BM_.*" 4)
add_filter_test(filter_suffix_negative "-BM_.*" 1)
add_filter_test(filter_regex_all ".*" 5)
add_filter_test(filter_regex_all_negative "-.*" 0)
add_filter_test(filter_regex_blank "" 5)
add_filter_test(filter_regex_blank_negative "-" 0)
add_filter_test(filter_regex_none "monkey" 0)
add_filter_test(filter_regex_none_negative "-monkey" 5)
add_filter_test(filter_regex_wildcard ".*Foo.*" 3)
add_filter_test(filter_regex_wildcard_negative "-.*Foo.*" 2)
add_filter_test(filter_regex_begin "^BM_.*" 4)
add_filter_test(filter_regex_begin_negative "-^BM_.*" 1)
add_filter_test(filter_regex_begin2 "^N" 1)
add_filter_test(filter_regex_begin2_negative "-^N" 4)
add_filter_test(filter_regex_end ".*Ba$" 1)
add_filter_test(filter_regex_end_negative "-.*Ba$" 4)

compile_benchmark_test(options_test)
add_test(options_benchmarks options_test --benchmark_min_time=0.01)

compile_benchmark_test(basic_test)
add_test(basic_benchmark basic_test --benchmark_min_time=0.01)

compile_benchmark_test(diagnostics_test)
add_test(diagnostics_test diagnostics_test --benchmark_min_time=0.01)

compile_benchmark_test(skip_with_error_test)
add_test(skip_with_error_test skip_with_error_test --benchmark_min_time=0.01)

compile_benchmark_test(donotoptimize_test)
# Some of the issues with DoNotOptimize only occur when optimization is enabled
check_cxx_compiler_flag(-O3 BENCHMARK_HAS_O3_FLAG)
if (BENCHMARK_HAS_O3_FLAG)
  set_target_properties(donotoptimize_test PROPERTIES COMPILE_FLAGS "-O3")
endif()
add_test(donotoptimize_test donotoptimize_test --benchmark_min_time=0.01)

compile_benchmark_test(fixture_test)
add_test(fixture_test fixture_test --benchmark_min_time=0.01)

compile_benchmark_test(register_benchmark_test)
add_test(register_benchmark_test register_benchmark_test --benchmark_min_time=0.01)

compile_benchmark_test(map_test)
add_test(map_test map_test --benchmark_min_time=0.01)

compile_benchmark_test(multiple_ranges_test)
add_test(multiple_ranges_test multiple_ranges_test --benchmark_min_time=0.01)

compile_benchmark_test_with_main(link_main_test)
add_test(link_main_test link_main_test --benchmark_min_time=0.01)

compile_output_test(reporter_output_test)
add_test(reporter_output_test reporter_output_test --benchmark_min_time=0.01)

compile_output_test(templated_fixture_test)
add_test(templated_fixture_test templated_fixture_test --benchmark_min_time=0.01)

compile_output_test(user_counters_test)
add_test(user_counters_test user_counters_test --benchmark_min_time=0.01)

compile_output_test(report_aggregates_only_test)
add_test(report_aggregates_only_test report_aggregates_only_test --benchmark_min_time=0.01)

compile_output_test(display_aggregates_only_test)
add_test(display_aggregates_only_test display_aggregates_only_test --benchmark_min_time=0.01)

compile_output_test(user_counters_tabular_test)
add_test(user_counters_tabular_test user_counters_tabular_test --benchmark_counters_tabular=true --benchmark_min_time=0.01)

compile_output_test(user_counters_thousands_test)
add_test(user_counters_thousands_test user_counters_thousands_test --benchmark_min_time=0.01)

compile_output_test(memory_manager_test)
add_test(memory_manager_test memory_manager_test --benchmark_min_time=0.01)

check_cxx_compiler_flag(-std=c++03 BENCHMARK_HAS_CXX03_FLAG)
if (BENCHMARK_HAS_CXX03_FLAG)
  compile_benchmark_test(cxx03_test)
  set_target_properties(cxx03_test
      PROPERTIES
      COMPILE_FLAGS "-std=c++03")
  # libstdc++ provides different definitions within <map> between dialects. When
  # LTO is enabled and -Werror is specified GCC diagnoses this ODR violation
  # causing the test to fail to compile. To prevent this we explicitly disable
  # the warning.
  check_cxx_compiler_flag(-Wno-odr BENCHMARK_HAS_WNO_ODR)
  if (BENCHMARK_ENABLE_LTO AND BENCHMARK_HAS_WNO_ODR)
    set_target_properties(cxx03_test
        PROPERTIES
        LINK_FLAGS "-Wno-odr")
  endif()
  add_test(cxx03 cxx03_test --benchmark_min_time=0.01)
endif()

# Attempt to work around flaky test failures when running on Appveyor servers.
if (DEFINED ENV{APPVEYOR})
  set(COMPLEXITY_MIN_TIME "0.5")
else()
  set(COMPLEXITY_MIN_TIME "0.01")
endif()
compile_output_test(complexity_test)
add_test(complexity_benchmark complexity_test --benchmark_min_time=${COMPLEXITY_MIN_TIME})

###############################################################################
# GoogleTest Unit Tests
###############################################################################

if (BENCHMARK_ENABLE_GTEST_TESTS)
  macro(compile_gtest name)
    add_executable(${name} "${name}.cc")
    if (TARGET googletest)
      add_dependencies(${name} googletest)
    endif()
    if (GTEST_INCLUDE_DIRS)
      target_include_directories(${name} PRIVATE ${GTEST_INCLUDE_DIRS})
    endif()
    target_link_libraries(${name} benchmark
        ${GTEST_BOTH_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
  endmacro(compile_gtest)

  macro(add_gtest name)
    compile_gtest(${name})
    add_test(${name} ${name})
  endmacro()

  add_gtest(benchmark_gtest)
  add_gtest(statistics_gtest)
  add_gtest(string_util_gtest)
endif(BENCHMARK_ENABLE_GTEST_TESTS)

###############################################################################
# Assembly Unit Tests
###############################################################################

if (BENCHMARK_ENABLE_ASSEMBLY_TESTS)
  if (NOT LLVM_FILECHECK_EXE)
    message(FATAL_ERROR "LLVM FileCheck is required when including this file")
  endif()
  include(AssemblyTests.cmake)
  add_filecheck_test(donotoptimize_assembly_test)
  add_filecheck_test(state_assembly_test)
  add_filecheck_test(clobber_memory_assembly_test)
endif()



###############################################################################
# Code Coverage Configuration
###############################################################################

# Add the coverage command(s)
if(CMAKE_BUILD_TYPE)
  string(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_LOWER)
endif()
if (${CMAKE_BUILD_TYPE_LOWER} MATCHES "coverage")
  find_program(GCOV gcov)
  find_program(LCOV lcov)
  find_program(GENHTML genhtml)
  find_program(CTEST ctest)
  if (GCOV AND LCOV AND GENHTML AND CTEST AND HAVE_CXX_FLAG_COVERAGE)
    add_custom_command(
      OUTPUT ${CMAKE_BINARY_DIR}/lcov/index.html
      COMMAND ${LCOV} -q -z -d .
      COMMAND ${LCOV} -q --no-external -c -b "${CMAKE_SOURCE_DIR}" -d . -o before.lcov -i
      COMMAND ${CTEST} --force-new-ctest-process
      COMMAND ${LCOV} -q --no-external -c -b "${CMAKE_SOURCE_DIR}" -d . -o after.lcov
      COMMAND ${LCOV} -q -a before.lcov -a after.lcov --output-file final.lcov
      COMMAND ${LCOV} -q -r final.lcov "'${CMAKE_SOURCE_DIR}/test/*'" -o final.lcov
      COMMAND ${GENHTML} final.lcov -o lcov --demangle-cpp --sort -p "${CMAKE_BINARY_DIR}" -t benchmark
      DEPENDS filter_test benchmark_test options_test basic_test fixture_test cxx03_test complexity_test
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      COMMENT "Running LCOV"
    )
    add_custom_target(coverage
      DEPENDS ${CMAKE_BINARY_DIR}/lcov/index.html
      COMMENT "LCOV report at lcov/index.html"
    )
    message(STATUS "Coverage command added")
  else()
    if (HAVE_CXX_FLAG_COVERAGE)
      set(CXX_FLAG_COVERAGE_MESSAGE supported)
    else()
      set(CXX_FLAG_COVERAGE_MESSAGE unavailable)
    endif()
    message(WARNING
      "Coverage not available:\n"
      "  gcov: ${GCOV}\n"
      "  lcov: ${LCOV}\n"
      "  genhtml: ${GENHTML}\n"
      "  ctest: ${CTEST}\n"
      "  --coverage flag: ${CXX_FLAG_COVERAGE_MESSAGE}")
  endif()
endif()
