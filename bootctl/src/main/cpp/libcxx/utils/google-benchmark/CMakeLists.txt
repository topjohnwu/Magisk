cmake_minimum_required (VERSION 2.8.12)

project (benchmark)

foreach(p
    CMP0054 # CMake 3.1
    CMP0056 # export EXE_LINKER_FLAGS to try_run
    CMP0057 # Support no if() IN_LIST operator
    )
  if(POLICY ${p})
    cmake_policy(SET ${p} NEW)
  endif()
endforeach()

option(BENCHMARK_ENABLE_TESTING "Enable testing of the benchmark library." ON)
option(BENCHMARK_ENABLE_EXCEPTIONS "Enable the use of exceptions in the benchmark library." ON)
option(BENCHMARK_ENABLE_LTO "Enable link time optimisation of the benchmark library." OFF)
option(BENCHMARK_USE_LIBCXX "Build and test using libc++ as the standard library." OFF)
if(NOT MSVC)
  option(BENCHMARK_BUILD_32_BITS "Build a 32 bit version of the library." OFF)
else()
  set(BENCHMARK_BUILD_32_BITS OFF CACHE BOOL "Build a 32 bit version of the library - unsupported when using MSVC)" FORCE)
endif()
option(BENCHMARK_ENABLE_INSTALL "Enable installation of benchmark. (Projects embedding benchmark may want to turn this OFF.)" ON)

# Allow unmet dependencies to be met using CMake's ExternalProject mechanics, which
# may require downloading the source code.
option(BENCHMARK_DOWNLOAD_DEPENDENCIES "Allow the downloading and in-tree building of unmet dependencies" OFF)

# This option can be used to disable building and running unit tests which depend on gtest
# in cases where it is not possible to build or find a valid version of gtest.
option(BENCHMARK_ENABLE_GTEST_TESTS "Enable building the unit tests which depend on gtest" ON)

set(ENABLE_ASSEMBLY_TESTS_DEFAULT OFF)
function(should_enable_assembly_tests)
  if(CMAKE_BUILD_TYPE)
    string(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_LOWER)
    if (${CMAKE_BUILD_TYPE_LOWER} MATCHES "coverage")
      # FIXME: The --coverage flag needs to be removed when building assembly
      # tests for this to work.
      return()
    endif()
  endif()
  if (MSVC)
    return()
  elseif(NOT CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
    return()
  elseif(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    # FIXME: Make these work on 32 bit builds
    return()
  elseif(BENCHMARK_BUILD_32_BITS)
     # FIXME: Make these work on 32 bit builds
    return()
  endif()
  find_program(LLVM_FILECHECK_EXE FileCheck)
  if (LLVM_FILECHECK_EXE)
    set(LLVM_FILECHECK_EXE "${LLVM_FILECHECK_EXE}" CACHE PATH "llvm filecheck" FORCE)
    message(STATUS "LLVM FileCheck Found: ${LLVM_FILECHECK_EXE}")
  else()
    message(STATUS "Failed to find LLVM FileCheck")
    return()
  endif()
  set(ENABLE_ASSEMBLY_TESTS_DEFAULT ON PARENT_SCOPE)
endfunction()
should_enable_assembly_tests()

# This option disables the building and running of the assembly verification tests
option(BENCHMARK_ENABLE_ASSEMBLY_TESTS "Enable building and running the assembly tests"
    ${ENABLE_ASSEMBLY_TESTS_DEFAULT})

# Make sure we can import out CMake functions
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")


# Read the git tags to determine the project version
include(GetGitVersion)
get_git_version(GIT_VERSION)

# Tell the user what versions we are using
string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" VERSION ${GIT_VERSION})
message(STATUS "Version: ${VERSION}")

# The version of the libraries
set(GENERIC_LIB_VERSION ${VERSION})
string(SUBSTRING ${VERSION} 0 1 GENERIC_LIB_SOVERSION)

# Import our CMake modules
include(CheckCXXCompilerFlag)
include(AddCXXCompilerFlag)
include(CXXFeatureCheck)

if (BENCHMARK_BUILD_32_BITS)
  add_required_cxx_compiler_flag(-m32)
endif()

if (MSVC)
  # Turn compiler warnings up to 11
  string(REGEX REPLACE "[-/]W[1-4]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)

  if (NOT BENCHMARK_ENABLE_EXCEPTIONS)
    add_cxx_compiler_flag(-EHs-)
    add_cxx_compiler_flag(-EHa-)
    add_definitions(-D_HAS_EXCEPTIONS=0)
  endif()
  # Link time optimisation
  if (BENCHMARK_ENABLE_LTO)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GL")
    set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "${CMAKE_STATIC_LINKER_FLAGS_RELEASE} /LTCG")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /LTCG")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")

    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /GL")
    string(REGEX REPLACE "[-/]INCREMENTAL" "/INCREMENTAL:NO" CMAKE_STATIC_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_STATIC_LINKER_FLAGS_RELWITHDEBINFO}")
    set(CMAKE_STATIC_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_STATIC_LINKER_FLAGS_RELWITHDEBINFO} /LTCG")
    string(REGEX REPLACE "[-/]INCREMENTAL" "/INCREMENTAL:NO" CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO}")
    set(CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO} /LTCG")
    string(REGEX REPLACE "[-/]INCREMENTAL" "/INCREMENTAL:NO" CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO}")
    set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} /LTCG")

    set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /GL")
    set(CMAKE_STATIC_LINKER_FLAGS_MINSIZEREL "${CMAKE_STATIC_LINKER_FLAGS_MINSIZEREL} /LTCG")
    set(CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL "${CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL} /LTCG")
    set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL "${CMAKE_EXE_LINKER_FLAGS_MINSIZEREL} /LTCG")
  endif()
else()
  # Try and enable C++11. Don't use C++14 because it doesn't work in some
  # configurations.
  add_cxx_compiler_flag(-std=c++11)
  if (NOT HAVE_CXX_FLAG_STD_CXX11)
    add_cxx_compiler_flag(-std=c++0x)
  endif()

  # Turn compiler warnings up to 11
  add_cxx_compiler_flag(-Wall)
  add_cxx_compiler_flag(-Wextra)
  add_cxx_compiler_flag(-Wshadow)
  add_cxx_compiler_flag(-Werror RELEASE)
  add_cxx_compiler_flag(-Werror RELWITHDEBINFO)
  add_cxx_compiler_flag(-Werror MINSIZEREL)
  add_cxx_compiler_flag(-pedantic)
  add_cxx_compiler_flag(-pedantic-errors)
  add_cxx_compiler_flag(-Wshorten-64-to-32)
  add_cxx_compiler_flag(-fstrict-aliasing)
  # Disable warnings regarding deprecated parts of the library while building
  # and testing those parts of the library.
  add_cxx_compiler_flag(-Wno-deprecated-declarations)
  if (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    # Intel silently ignores '-Wno-deprecated-declarations',
    # warning no. 1786 must be explicitly disabled.
    # See #631 for rationale.
    add_cxx_compiler_flag(-wd1786)
  endif()
  # Disable deprecation warnings for release builds (when -Werror is enabled).
  add_cxx_compiler_flag(-Wno-deprecated RELEASE)
  add_cxx_compiler_flag(-Wno-deprecated RELWITHDEBINFO)
  add_cxx_compiler_flag(-Wno-deprecated MINSIZEREL)
  if (NOT BENCHMARK_ENABLE_EXCEPTIONS)
    add_cxx_compiler_flag(-fno-exceptions)
  endif()

  if (HAVE_CXX_FLAG_FSTRICT_ALIASING)
    if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "Intel") #ICC17u2: Many false positives for Wstrict-aliasing
      add_cxx_compiler_flag(-Wstrict-aliasing)
    endif()
  endif()
  # ICC17u2: overloaded virtual function "benchmark::Fixture::SetUp" is only partially overridden
  # (because of deprecated overload)
  add_cxx_compiler_flag(-wd654)
  add_cxx_compiler_flag(-Wthread-safety)
  if (HAVE_CXX_FLAG_WTHREAD_SAFETY)
    cxx_feature_check(THREAD_SAFETY_ATTRIBUTES)
  endif()

  # On most UNIX like platforms g++ and clang++ define _GNU_SOURCE as a
  # predefined macro, which turns on all of the wonderful libc extensions.
  # However g++ doesn't do this in Cygwin so we have to define it ourselfs
  # since we depend on GNU/POSIX/BSD extensions.
  if (CYGWIN)
    add_definitions(-D_GNU_SOURCE=1)
  endif()

  # Link time optimisation
  if (BENCHMARK_ENABLE_LTO)
    add_cxx_compiler_flag(-flto)
    if ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
      find_program(GCC_AR gcc-ar)
      if (GCC_AR)
        set(CMAKE_AR ${GCC_AR})
      endif()
      find_program(GCC_RANLIB gcc-ranlib)
      if (GCC_RANLIB)
        set(CMAKE_RANLIB ${GCC_RANLIB})
      endif()
    elseif("${CMAKE_C_COMPILER_ID}" MATCHES "Clang")
      include(llvm-toolchain)
    endif()
  endif()

  # Coverage build type
  set(BENCHMARK_CXX_FLAGS_COVERAGE "${CMAKE_CXX_FLAGS_DEBUG}"
    CACHE STRING "Flags used by the C++ compiler during coverage builds."
    FORCE)
  set(BENCHMARK_EXE_LINKER_FLAGS_COVERAGE "${CMAKE_EXE_LINKER_FLAGS_DEBUG}"
    CACHE STRING "Flags used for linking binaries during coverage builds."
    FORCE)
  set(BENCHMARK_SHARED_LINKER_FLAGS_COVERAGE "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}"
    CACHE STRING "Flags used by the shared libraries linker during coverage builds."
    FORCE)
  mark_as_advanced(
    BENCHMARK_CXX_FLAGS_COVERAGE
    BENCHMARK_EXE_LINKER_FLAGS_COVERAGE
    BENCHMARK_SHARED_LINKER_FLAGS_COVERAGE)
  set(CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING
    "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel Coverage.")
  add_cxx_compiler_flag(--coverage COVERAGE)
endif()

if (BENCHMARK_USE_LIBCXX)
  if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    add_cxx_compiler_flag(-stdlib=libc++)
  elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR
          "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
    add_cxx_compiler_flag(-nostdinc++)
    message(WARNING "libc++ header path must be manually specified using CMAKE_CXX_FLAGS")
    # Adding -nodefaultlibs directly to CMAKE_<TYPE>_LINKER_FLAGS will break
    # configuration checks such as 'find_package(Threads)'
    list(APPEND BENCHMARK_CXX_LINKER_FLAGS -nodefaultlibs)
    # -lc++ cannot be added directly to CMAKE_<TYPE>_LINKER_FLAGS because
    # linker flags appear before all linker inputs and -lc++ must appear after.
    list(APPEND BENCHMARK_CXX_LIBRARIES c++)
  else()
    message(FATAL_ERROR "-DBENCHMARK_USE_LIBCXX:BOOL=ON is not supported for compiler")
  endif()
endif(BENCHMARK_USE_LIBCXX)

# C++ feature checks
# Determine the correct regular expression engine to use
cxx_feature_check(STD_REGEX)
cxx_feature_check(GNU_POSIX_REGEX)
cxx_feature_check(POSIX_REGEX)
if(NOT HAVE_STD_REGEX AND NOT HAVE_GNU_POSIX_REGEX AND NOT HAVE_POSIX_REGEX)
  message(FATAL_ERROR "Failed to determine the source files for the regular expression backend")
endif()
if (NOT BENCHMARK_ENABLE_EXCEPTIONS AND HAVE_STD_REGEX
        AND NOT HAVE_GNU_POSIX_REGEX AND NOT HAVE_POSIX_REGEX)
  message(WARNING "Using std::regex with exceptions disabled is not fully supported")
endif()
cxx_feature_check(STEADY_CLOCK)
# Ensure we have pthreads
find_package(Threads REQUIRED)

# Set up directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Build the targets
add_subdirectory(src)

if (BENCHMARK_ENABLE_TESTING)
  enable_testing()
  if (BENCHMARK_ENABLE_GTEST_TESTS)
    include(HandleGTest)
  endif()
  add_subdirectory(test)
endif()
