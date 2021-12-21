
include(split_list)

macro(build_external_gtest)
  include(ExternalProject)
  set(GTEST_FLAGS "")
  if (BENCHMARK_USE_LIBCXX)
    if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
      list(APPEND GTEST_FLAGS -stdlib=libc++)
    else()
      message(WARNING "Unsupported compiler (${CMAKE_CXX_COMPILER}) when using libc++")
    endif()
  endif()
  if (BENCHMARK_BUILD_32_BITS)
    list(APPEND GTEST_FLAGS -m32)
  endif()
  if (NOT "${CMAKE_CXX_FLAGS}" STREQUAL "")
    list(APPEND GTEST_FLAGS ${CMAKE_CXX_FLAGS})
  endif()
  string(TOUPPER "${CMAKE_BUILD_TYPE}" GTEST_BUILD_TYPE)
  if ("${GTEST_BUILD_TYPE}" STREQUAL "COVERAGE")
    set(GTEST_BUILD_TYPE "DEBUG")
  endif()
  # FIXME: Since 10/Feb/2017 the googletest trunk has had a bug where
  # -Werror=unused-function fires during the build on OS X. This is a temporary
  # workaround to keep our travis bots from failing. It should be removed
  # once gtest is fixed.
  if (NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    list(APPEND GTEST_FLAGS "-Wno-unused-function")
  endif()
  split_list(GTEST_FLAGS)
  set(EXCLUDE_FROM_ALL_OPT "")
  set(EXCLUDE_FROM_ALL_VALUE "")
  if (${CMAKE_VERSION} VERSION_GREATER "3.0.99")
      set(EXCLUDE_FROM_ALL_OPT "EXCLUDE_FROM_ALL")
      set(EXCLUDE_FROM_ALL_VALUE "ON")
  endif()
  ExternalProject_Add(googletest
      ${EXCLUDE_FROM_ALL_OPT} ${EXCLUDE_FROM_ALL_VALUE}
      GIT_REPOSITORY https://github.com/google/googletest.git
      GIT_TAG master
      PREFIX "${CMAKE_BINARY_DIR}/googletest"
      INSTALL_DIR "${CMAKE_BINARY_DIR}/googletest"
      CMAKE_CACHE_ARGS
        -DCMAKE_BUILD_TYPE:STRING=${GTEST_BUILD_TYPE}
        -DCMAKE_C_COMPILER:STRING=${CMAKE_C_COMPILER}
        -DCMAKE_CXX_COMPILER:STRING=${CMAKE_CXX_COMPILER}
        -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
        -DCMAKE_INSTALL_LIBDIR:PATH=<INSTALL_DIR>/lib
        -DCMAKE_CXX_FLAGS:STRING=${GTEST_FLAGS}
        -Dgtest_force_shared_crt:BOOL=ON
      )

  ExternalProject_Get_Property(googletest install_dir)
  set(GTEST_INCLUDE_DIRS ${install_dir}/include)
  file(MAKE_DIRECTORY ${GTEST_INCLUDE_DIRS})

  set(LIB_SUFFIX "${CMAKE_STATIC_LIBRARY_SUFFIX}")
  set(LIB_PREFIX "${CMAKE_STATIC_LIBRARY_PREFIX}")
  if("${GTEST_BUILD_TYPE}" STREQUAL "DEBUG")
    set(LIB_SUFFIX "d${CMAKE_STATIC_LIBRARY_SUFFIX}")
  endif()

  # Use gmock_main instead of gtest_main because it initializes gtest as well.
  # Note: The libraries are listed in reverse order of their dependancies.
  foreach(LIB gtest gmock gmock_main)
    add_library(${LIB} UNKNOWN IMPORTED)
    set_target_properties(${LIB} PROPERTIES
      IMPORTED_LOCATION ${install_dir}/lib/${LIB_PREFIX}${LIB}${LIB_SUFFIX}
      INTERFACE_INCLUDE_DIRECTORIES ${GTEST_INCLUDE_DIRS}
      INTERFACE_LINK_LIBRARIES "${GTEST_BOTH_LIBRARIES}"
    )
    add_dependencies(${LIB} googletest)
    list(APPEND GTEST_BOTH_LIBRARIES ${LIB})
  endforeach()
endmacro(build_external_gtest)

if (BENCHMARK_ENABLE_GTEST_TESTS)
  if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/googletest)
    set(GTEST_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/googletest")
    set(INSTALL_GTEST OFF CACHE INTERNAL "")
    set(INSTALL_GMOCK OFF CACHE INTERNAL "")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/googletest)
    set(GTEST_BOTH_LIBRARIES gtest gmock gmock_main)
    foreach(HEADER test mock)
      # CMake 2.8 and older don't respect INTERFACE_INCLUDE_DIRECTORIES, so we
      # have to add the paths ourselves.
      set(HFILE g${HEADER}/g${HEADER}.h)
      set(HPATH ${GTEST_ROOT}/google${HEADER}/include)
      find_path(HEADER_PATH_${HEADER} ${HFILE}
          NO_DEFAULT_PATHS
          HINTS ${HPATH}
      )
      if (NOT HEADER_PATH_${HEADER})
        message(FATAL_ERROR "Failed to find header ${HFILE} in ${HPATH}")
      endif()
      list(APPEND GTEST_INCLUDE_DIRS ${HEADER_PATH_${HEADER}})
    endforeach()
  elseif(BENCHMARK_DOWNLOAD_DEPENDENCIES)
    build_external_gtest()
  else()
    find_package(GTest REQUIRED)
    find_path(GMOCK_INCLUDE_DIRS gmock/gmock.h
        HINTS ${GTEST_INCLUDE_DIRS})
    if (NOT GMOCK_INCLUDE_DIRS)
      message(FATAL_ERROR "Failed to find header gmock/gmock.h with hint ${GTEST_INCLUDE_DIRS}")
    endif()
    set(GTEST_INCLUDE_DIRS ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS})
    # FIXME: We don't currently require the gmock library to build the tests,
    # and it's likely we won't find it, so we don't try. As long as we've
    # found the gmock/gmock.h header and gtest_main that should be good enough.
  endif()
endif()
