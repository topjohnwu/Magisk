set(LIBCXX_LIB_CMAKEFILES_DIR "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}"  PARENT_SCOPE)

# Get sources
# FIXME: Don't use glob here
file(GLOB LIBCXX_SOURCES ../src/*.cpp)
if(WIN32)
  file(GLOB LIBCXX_WIN32_SOURCES ../src/support/win32/*.cpp)
  list(APPEND LIBCXX_SOURCES ${LIBCXX_WIN32_SOURCES})
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "SunOS")
  file(GLOB LIBCXX_SOLARIS_SOURCES ../src/support/solaris/*.cpp)
  list(APPEND LIBCXX_SOURCES ${LIBCXX_SOLARIS_SOURCES})
endif()

# Add all the headers to the project for IDEs.
if (LIBCXX_CONFIGURE_IDE)
  file(GLOB_RECURSE LIBCXX_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/../include/*)
  if(WIN32)
    file( GLOB LIBCXX_WIN32_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/../include/support/win32/*.h)
    list(APPEND LIBCXX_HEADERS ${LIBCXX_WIN32_HEADERS})
  endif()
  # Force them all into the headers dir on MSVC, otherwise they end up at
  # project scope because they don't have extensions.
  if (MSVC_IDE)
    source_group("Header Files" FILES ${LIBCXX_HEADERS})
  endif()
endif()

if(NOT LIBCXX_INSTALL_LIBRARY)
  set(exclude_from_all EXCLUDE_FROM_ALL)
endif()

# If LIBCXX_CXX_ABI_LIBRARY_PATH is defined we want to add it to the search path.
add_link_flags_if(LIBCXX_CXX_ABI_LIBRARY_PATH
                  "${CMAKE_LIBRARY_PATH_FLAG}${LIBCXX_CXX_ABI_LIBRARY_PATH}")


if (LIBCXX_GENERATE_COVERAGE AND NOT LIBCXX_COVERAGE_LIBRARY)
  find_compiler_rt_library(profile LIBCXX_COVERAGE_LIBRARY)
endif()
add_library_flags_if(LIBCXX_COVERAGE_LIBRARY "${LIBCXX_COVERAGE_LIBRARY}")

if (APPLE AND (LIBCXX_CXX_ABI_LIBNAME STREQUAL "libcxxabi" OR
               LIBCXX_CXX_ABI_LIBNAME STREQUAL "default"))
  set(LIBCXX_OSX_REEXPORT_LIBCXXABI_SYMBOLS ON)
endif()

if (LIBCXX_STATICALLY_LINK_ABI_IN_SHARED_LIBRARY)
  add_library_flags("-Wl,--whole-archive" "-Wl,-Bstatic")
  add_library_flags("${LIBCXX_CXX_ABI_LIBRARY}")
  add_library_flags("-Wl,-Bdynamic" "-Wl,--no-whole-archive")
elseif (LIBCXX_OSX_REEXPORT_LIBCXXABI_SYMBOLS)
  add_library_flags("${LIBCXX_CXX_ABI_LIBRARY}")
else ()
 add_interface_library("${LIBCXX_CXX_ABI_LIBRARY}")
endif()

if (APPLE AND LLVM_USE_SANITIZER)
  if (("${LLVM_USE_SANITIZER}" STREQUAL "Address") OR
      ("${LLVM_USE_SANITIZER}" STREQUAL "Address;Undefined") OR
      ("${LLVM_USE_SANITIZER}" STREQUAL "Undefined;Address"))
    set(LIBFILE "libclang_rt.asan_osx_dynamic.dylib")
  elseif("${LLVM_USE_SANITIZER}" STREQUAL "Undefined")
    set(LIBFILE "libclang_rt.ubsan_osx_dynamic.dylib")
  elseif("${LLVM_USE_SANITIZER}" STREQUAL "Thread")
    set(LIBFILE "libclang_rt.tsan_osx_dynamic.dylib")
  else()
    message(WARNING "LLVM_USE_SANITIZER=${LLVM_USE_SANITIZER} is not supported on OS X")
  endif()
  if (LIBFILE)
    find_compiler_rt_dir(LIBDIR)
    if (NOT IS_DIRECTORY "${LIBDIR}")
      message(FATAL_ERROR "Cannot find compiler-rt directory on OS X required for LLVM_USE_SANITIZER")
    endif()
    set(LIBCXX_SANITIZER_LIBRARY "${LIBDIR}/${LIBFILE}")
    set(LIBCXX_SANITIZER_LIBRARY "${LIBCXX_SANITIZER_LIBRARY}" PARENT_SCOPE)
    message(STATUS "Manually linking compiler-rt library: ${LIBCXX_SANITIZER_LIBRARY}")
    add_library_flags("${LIBCXX_SANITIZER_LIBRARY}")
    add_link_flags("-Wl,-rpath,${LIBDIR}")
  endif()
endif()

# Generate private library list.
add_library_flags_if(LIBCXX_HAS_PTHREAD_LIB pthread)
add_library_flags_if(LIBCXX_HAS_C_LIB c)
add_library_flags_if(LIBCXX_HAS_M_LIB m)
add_library_flags_if(LIBCXX_HAS_RT_LIB rt)
if (LIBCXX_USE_COMPILER_RT)
  find_compiler_rt_library(builtins LIBCXX_BUILTINS_LIBRARY)
  add_library_flags_if(LIBCXX_BUILTINS_LIBRARY "${LIBCXX_BUILTINS_LIBRARY}")
else()
  add_library_flags_if(LIBCXX_HAS_GCC_S_LIB gcc_s)
endif()
add_library_flags_if(LIBCXX_HAVE_CXX_ATOMICS_WITH_LIB atomic)
add_library_flags_if(MINGW "${MINGW_LIBRARIES}")

# Add the unwinder library.
if (LIBCXXABI_USE_LLVM_UNWINDER)
  if (NOT LIBCXXABI_ENABLE_STATIC_UNWINDER AND (TARGET unwind_shared OR HAVE_LIBUNWIND))
    add_interface_library(unwind_shared)
  elseif (LIBCXXABI_ENABLE_STATIC_UNWINDER AND (TARGET unwind_static OR HAVE_LIBUNWIND))
    add_interface_library(unwind_static)
  else()
    add_interface_library(unwind)
  endif()
endif()

# Setup flags.
if (NOT WIN32)
  add_flags_if_supported(-fPIC)
endif()

add_link_flags_if_supported(-nodefaultlibs)

if (LIBCXX_TARGETING_MSVC)
  if (LIBCXX_DEBUG_BUILD)
    set(LIB_SUFFIX "d")
  else()
    set(LIB_SUFFIX "")
  endif()
  add_compile_flags(/Zl)
  add_link_flags(/nodefaultlib)

  add_library_flags(ucrt${LIB_SUFFIX}) # Universal C runtime
  add_library_flags(vcruntime${LIB_SUFFIX}) # C++ runtime
  add_library_flags(msvcrt${LIB_SUFFIX}) # C runtime startup files
  add_library_flags(msvcprt${LIB_SUFFIX}) # C++ standard library. Required for exception_ptr internals.
  # Required for standards-complaint wide character formatting functions
  # (e.g. `printfw`/`scanfw`)
  add_library_flags(iso_stdio_wide_specifiers)
endif()

if (LIBCXX_OSX_REEXPORT_LIBCXXABI_SYMBOLS)
  if (NOT DEFINED LIBCXX_LIBCPPABI_VERSION)
    set(LIBCXX_LIBCPPABI_VERSION "2") # Default value
    execute_process(
      COMMAND xcrun --show-sdk-version
      OUTPUT_VARIABLE sdk_ver
      RESULT_VARIABLE res
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (res EQUAL 0)
      message(STATUS "Found SDK version ${sdk_ver}")
      string(REPLACE "10." "" sdk_ver "${sdk_ver}")
      if (sdk_ver LESS 9)
        set(LIBCXX_LIBCPPABI_VERSION "")
      else()
        set(LIBCXX_LIBCPPABI_VERSION "2")
      endif()
    endif()
  endif()

  if ( CMAKE_OSX_DEPLOYMENT_TARGET STREQUAL "10.6" )
    message(FATAL_ERROR "Mac OSX 10.6 is not supported anymore as a deployment "
                        "target. If you need support for this, please contact "
                        "the libc++ maintainers.")
  else()
    if ("armv7" IN_LIST CMAKE_OSX_ARCHITECTURES)
      set(RE_EXPORT_LIST "${CMAKE_CURRENT_SOURCE_DIR}/libc++sjlj-abi.exp")
    else()
      set(RE_EXPORT_LIST "${CMAKE_CURRENT_SOURCE_DIR}/libc++abi${LIBCXX_LIBCPPABI_VERSION}.exp")
    endif()
    add_link_flags(
      "-compatibility_version 1"
      "-install_name /usr/lib/libc++.1.dylib"
      "-Wl,-unexported_symbols_list,\"${CMAKE_CURRENT_SOURCE_DIR}/libc++unexp.exp\""
      "-Wl,-reexported_symbols_list,\"${RE_EXPORT_LIST}\""
      "-Wl,-force_symbols_not_weak_list,\"${CMAKE_CURRENT_SOURCE_DIR}/notweak.exp\""
      "-Wl,-force_symbols_weak_list,\"${CMAKE_CURRENT_SOURCE_DIR}/weak.exp\"")

    if (NOT LIBCXX_ENABLE_NEW_DELETE_DEFINITIONS)
      add_link_flags("-Wl,-reexported_symbols_list,\"${CMAKE_CURRENT_SOURCE_DIR}/libc++abi-new-delete.exp\"")
    endif()
  endif()
endif()

split_list(LIBCXX_COMPILE_FLAGS)
split_list(LIBCXX_LINK_FLAGS)

macro(cxx_object_library name)
  cmake_parse_arguments(ARGS "" "" "DEFINES;FLAGS" ${ARGN})

  # Add an object library that contains the compiled source files.
  add_library(${name} OBJECT ${exclude_from_all} ${LIBCXX_SOURCES} ${LIBCXX_HEADERS})
  if(LIBCXX_CXX_ABI_HEADER_TARGET)
    add_dependencies(${name} ${LIBCXX_CXX_ABI_HEADER_TARGET})
  endif()
  if(WIN32 AND NOT MINGW)
    target_compile_definitions(${name}
                               PRIVATE
                                 # Ignore the -MSC_VER mismatch, as we may build
                                 # with a different compatibility version.
                                 _ALLOW_MSC_VER_MISMATCH
                                 # Don't check the msvcprt iterator debug levels
                                 # as we will define the iterator types; libc++
                                 # uses a different macro to identify the debug
                                 # level.
                                 _ALLOW_ITERATOR_DEBUG_LEVEL_MISMATCH
                                 # We are building the c++ runtime, don't pull in
                                 # msvcprt.
                                 _CRTBLD
                                 # Don't warn on the use of "deprecated"
                                 # "insecure" functions which are standards
                                 # specified.
                                 _CRT_SECURE_NO_WARNINGS
                                 # Use the ISO conforming behaviour for conversion
                                 # in printf, scanf.
                                 _CRT_STDIO_ISO_WIDE_SPECIFIERS)
  endif()

  if(ARGS_DEFINES)
    target_compile_definitions(${name} PRIVATE ${ARGS_DEFINES})
  endif()

  set_target_properties(${name}
    PROPERTIES
      COMPILE_FLAGS ${LIBCXX_COMPILE_FLAGS}
  )

  if(ARGS_FLAGS)
    target_compile_options(${name} PRIVATE ${ARGS_FLAGS})
  endif()
endmacro()

if(LIBCXX_HERMETIC_STATIC_LIBRARY)
  append_flags_if_supported(CXX_STATIC_OBJECTS_FLAGS -fvisibility=hidden)
  append_flags_if_supported(CXX_STATIC_OBJECTS_FLAGS -fvisibility-global-new-delete-hidden)
  cxx_object_library(cxx_static_objects
    DEFINES _LIBCPP_DISABLE_VISIBILITY_ANNOTATIONS
    FLAGS ${CXX_STATIC_OBJECTS_FLAGS})
  cxx_object_library(cxx_shared_objects)
  set(cxx_static_sources $<TARGET_OBJECTS:cxx_static_objects>)
  set(cxx_shared_sources $<TARGET_OBJECTS:cxx_shared_objects>)
else()
  cxx_object_library(cxx_objects)
  set(cxx_static_sources $<TARGET_OBJECTS:cxx_objects>)
  set(cxx_shared_sources $<TARGET_OBJECTS:cxx_objects>)
endif()

# Build the shared library.
if (LIBCXX_ENABLE_SHARED)
  add_library(cxx_shared SHARED ${cxx_shared_sources})
  if(COMMAND llvm_setup_rpath)
    llvm_setup_rpath(cxx_shared)
  endif()
  target_link_libraries(cxx_shared ${LIBCXX_LIBRARIES})
  set_target_properties(cxx_shared
    PROPERTIES
      LINK_FLAGS    "${LIBCXX_LINK_FLAGS}"
      OUTPUT_NAME   "c++"
      VERSION       "${LIBCXX_ABI_VERSION}.0"
      SOVERSION     "${LIBCXX_ABI_VERSION}"
  )
  list(APPEND LIBCXX_BUILD_TARGETS "cxx_shared")
  if (LIBCXX_INSTALL_SHARED_LIBRARY)
    list(APPEND LIBCXX_INSTALL_TARGETS "cxx_shared")
  endif()
  if(WIN32 AND NOT MINGW AND NOT "${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Windows")
    # Since we most likely do not have a mt.exe replacement, disable the
    # manifest bundling.  This allows a normal cmake invocation to pass which
    # will attempt to use the manifest tool to generate the bundled manifest
    set_target_properties(cxx_shared PROPERTIES
                          APPEND_STRING PROPERTY LINK_FLAGS " /MANIFEST:NO")
  endif()
endif()

# Build the static library.
if (LIBCXX_ENABLE_STATIC)
  add_library(cxx_static STATIC ${cxx_static_sources})
  target_link_libraries(cxx_static ${LIBCXX_LIBRARIES})
  set(CMAKE_STATIC_LIBRARY_PREFIX "lib")
  set_target_properties(cxx_static
    PROPERTIES
      LINK_FLAGS    "${LIBCXX_LINK_FLAGS}"
      OUTPUT_NAME   "c++"
  )
  list(APPEND LIBCXX_BUILD_TARGETS "cxx_static")
  if (LIBCXX_INSTALL_STATIC_LIBRARY)
    list(APPEND LIBCXX_INSTALL_TARGETS "cxx_static")
  endif()
  # Attempt to merge the libc++.a archive and the ABI library archive into one.
  if (LIBCXX_STATICALLY_LINK_ABI_IN_STATIC_LIBRARY)
    set(MERGE_ARCHIVES_SEARCH_PATHS "")
    if (LIBCXX_CXX_ABI_LIBRARY_PATH)
      set(MERGE_ARCHIVES_SEARCH_PATHS "-L${LIBCXX_CXX_ABI_LIBRARY_PATH}")
    endif()
    if ((TARGET ${LIBCXX_CXX_ABI_LIBRARY}) OR
        (${LIBCXX_CXX_ABI_LIBRARY} MATCHES "cxxabi(_static|_shared)?" AND HAVE_LIBCXXABI))
      set(MERGE_ARCHIVES_ABI_TARGET "$<TARGET_LINKER_FILE:cxxabi_static>")
    else()
      set(MERGE_ARCHIVES_ABI_TARGET
          "${CMAKE_STATIC_LIBRARY_PREFIX}${LIBCXX_CXX_ABI_LIBRARY}${CMAKE_STATIC_LIBRARY_SUFFIX}")
    endif()
    add_custom_command(TARGET cxx_static POST_BUILD
    COMMAND
      ${PYTHON_EXECUTABLE} ${LIBCXX_SOURCE_DIR}/utils/merge_archives.py
    ARGS
      -o $<TARGET_LINKER_FILE:cxx_static>
      --ar "${CMAKE_AR}"
      "$<TARGET_LINKER_FILE:cxx_static>"
      "${MERGE_ARCHIVES_ABI_TARGET}"
      "${MERGE_ARCHIVES_SEARCH_PATHS}"
    WORKING_DIRECTORY ${LIBCXX_BUILD_DIR}
    )
  endif()
endif()

# Add a meta-target for both libraries.
add_custom_target(cxx DEPENDS cxx-headers ${LIBCXX_BUILD_TARGETS})

if (LIBCXX_ENABLE_FILESYSTEM)
  set(LIBCXX_FILESYSTEM_SOURCES
      ../src/filesystem/operations.cpp
      ../src/filesystem/directory_iterator.cpp)

  # Filesystem uses __int128_t, which requires a definition of __muloi4 when
  # compiled with UBSAN. This definition is not provided by libgcc_s, but is
  # provided by compiler-rt. So we need to disable it to avoid having multiple
  # definitions. See filesystem/int128_builtins.cpp.
  if (NOT LIBCXX_USE_COMPILER_RT)
    list(APPEND LIBCXX_FILESYSTEM_SOURCES ../src/filesystem/int128_builtins.cpp)
  endif()

  add_library(cxx_filesystem STATIC ${LIBCXX_FILESYSTEM_SOURCES})
  if (LIBCXX_ENABLE_SHARED)
    target_link_libraries(cxx_filesystem cxx_shared)
  else()
    target_link_libraries(cxx_filesystem cxx_static)
  endif()

  set(filesystem_flags "${LIBCXX_COMPILE_FLAGS}")
  check_flag_supported(-std=c++14)
  if (NOT MSVC AND LIBCXX_SUPPORTS_STD_EQ_CXX14_FLAG)
    string(REPLACE "-std=c++11" "-std=c++14" filesystem_flags "${LIBCXX_COMPILE_FLAGS}")
  endif()
  set_target_properties(cxx_filesystem
    PROPERTIES
      COMPILE_FLAGS "${filesystem_flags}"
      OUTPUT_NAME   "c++fs"
  )
endif()


if (LIBCXX_ENABLE_EXPERIMENTAL_LIBRARY)
  file(GLOB LIBCXX_EXPERIMENTAL_SOURCES ../src/experimental/*.cpp)
  add_library(cxx_experimental STATIC ${LIBCXX_EXPERIMENTAL_SOURCES})
  if (LIBCXX_ENABLE_SHARED)
    target_link_libraries(cxx_experimental cxx_shared)
  else()
    target_link_libraries(cxx_experimental cxx_static)
  endif()

  set(experimental_flags "${LIBCXX_COMPILE_FLAGS}")
  check_flag_supported(-std=c++14)
  if (NOT MSVC AND LIBCXX_SUPPORTS_STD_EQ_CXX14_FLAG)
    string(REPLACE "-std=c++11" "-std=c++14" experimental_flags "${LIBCXX_COMPILE_FLAGS}")
  endif()
  set_target_properties(cxx_experimental
    PROPERTIES
      COMPILE_FLAGS "${experimental_flags}"
      OUTPUT_NAME   "c++experimental"
  )
endif()

if (LIBCXX_BUILD_EXTERNAL_THREAD_LIBRARY)
  file(GLOB LIBCXX_EXTERNAL_THREADING_SUPPORT_SOURCES ../test/support/external_threads.cpp)

  if (LIBCXX_ENABLE_SHARED)
    add_library(cxx_external_threads SHARED ${LIBCXX_EXTERNAL_THREADING_SUPPORT_SOURCES})
  else()
    add_library(cxx_external_threads STATIC ${LIBCXX_EXTERNAL_THREADING_SUPPORT_SOURCES})
  endif()

  set_target_properties(cxx_external_threads
    PROPERTIES
      LINK_FLAGS    "${LIBCXX_LINK_FLAGS}"
      COMPILE_FLAGS "${LIBCXX_COMPILE_FLAGS}"
      OUTPUT_NAME   "c++external_threads"
  )
endif()

# Generate a linker script inplace of a libc++.so symlink. Rerun this command
# after cxx builds.
if (LIBCXX_ENABLE_SHARED AND LIBCXX_ENABLE_ABI_LINKER_SCRIPT)
  # Get the name of the ABI library and handle the case where CXXABI_LIBNAME
  # is a target name and not a library. Ex cxxabi_shared.
  set(LIBCXX_INTERFACE_LIBRARY_NAMES)
  foreach(lib ${LIBCXX_INTERFACE_LIBRARIES})
    # FIXME: Handle cxxabi_static and unwind_static.
    if (TARGET ${lib} OR
        (${lib} MATCHES "cxxabi(_static|_shared)?" AND HAVE_LIBCXXABI) OR
        (${lib} MATCHES "unwind(_static|_shared)?" AND HAVE_LIBUNWIND))
      list(APPEND LIBCXX_INTERFACE_LIBRARY_NAMES "$<TARGET_PROPERTY:${lib},OUTPUT_NAME>")
    else()
      list(APPEND LIBCXX_INTERFACE_LIBRARY_NAMES "${lib}")
    endif()
  endforeach()
  #split_list(LIBCXX_INTERFACE_LIBRARY_NAMES)
  # Generate a linker script inplace of a libc++.so symlink. Rerun this command
  # after cxx builds.
  add_custom_command(TARGET cxx_shared POST_BUILD
    COMMAND
      ${PYTHON_EXECUTABLE} ${LIBCXX_SOURCE_DIR}/utils/gen_link_script.py
    ARGS
      "$<TARGET_LINKER_FILE:cxx_shared>"
      ${LIBCXX_INTERFACE_LIBRARY_NAMES}
    WORKING_DIRECTORY ${LIBCXX_BUILD_DIR}
  )
endif()

if (LIBCXX_INSTALL_LIBRARY)
  if (LIBCXX_INSTALL_FILESYSTEM_LIBRARY)
    set(filesystem_lib cxx_filesystem)
  endif()
  if (LIBCXX_INSTALL_EXPERIMENTAL_LIBRARY)
    set(experimental_lib cxx_experimental)
  endif()
  install(TARGETS ${LIBCXX_INSTALL_TARGETS} ${filesystem_lib} ${experimental_lib}
    LIBRARY DESTINATION ${LIBCXX_INSTALL_PREFIX}lib${LIBCXX_LIBDIR_SUFFIX} COMPONENT cxx
    ARCHIVE DESTINATION ${LIBCXX_INSTALL_PREFIX}lib${LIBCXX_LIBDIR_SUFFIX} COMPONENT cxx
    )
  # NOTE: This install command must go after the cxx install command otherwise
  # it will not be executed after the library symlinks are installed.
  if (LIBCXX_ENABLE_SHARED AND LIBCXX_ENABLE_ABI_LINKER_SCRIPT)
    # Replace the libc++ filename with $<TARGET_LINKER_FILE:cxx>
    # after we required CMake 3.0.
    install(FILES "${LIBCXX_LIBRARY_DIR}/libc++${CMAKE_SHARED_LIBRARY_SUFFIX}"
      DESTINATION ${LIBCXX_INSTALL_PREFIX}lib${LIBCXX_LIBDIR_SUFFIX}
      COMPONENT libcxx)
  endif()
endif()

if (NOT CMAKE_CONFIGURATION_TYPES AND (LIBCXX_INSTALL_LIBRARY OR
                                       LIBCXX_INSTALL_HEADERS))
    if(LIBCXX_INSTALL_LIBRARY)
      set(lib_install_target cxx)
    endif()
    if (LIBCXX_INSTALL_FILESYSTEM_LIBRARY)
      set(filesystem_lib_install_target cxx_filesystem)
    endif()
    if (LIBCXX_INSTALL_EXPERIMENTAL_LIBRARY)
      set(experimental_lib_install_target cxx_experimental)
    endif()
    if(LIBCXX_INSTALL_HEADERS)
      set(header_install_target install-cxx-headers)
    endif()
    add_custom_target(install-cxx
                      DEPENDS ${lib_install_target}
                              ${experimental_lib_install_target}
                              ${filesystem_lib_install_target}
                              ${header_install_target}
                      COMMAND "${CMAKE_COMMAND}"
                      -DCMAKE_INSTALL_COMPONENT=cxx
                      -P "${LIBCXX_BINARY_DIR}/cmake_install.cmake")
    add_custom_target(install-cxx-stripped
                      DEPENDS ${lib_install_target}
                              ${experimental_lib_install_target}
                              ${filesystem_lib_install_target}
                              ${header_install_target}
                      COMMAND "${CMAKE_COMMAND}"
                      -DCMAKE_INSTALL_COMPONENT=cxx
                      -DCMAKE_INSTALL_DO_STRIP=1
                      -P "${LIBCXX_BINARY_DIR}/cmake_install.cmake")
    add_custom_target(install-libcxx DEPENDS install-cxx)
endif()
