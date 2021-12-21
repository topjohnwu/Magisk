# Get sources
set(LIBCXXABI_SOURCES
  # C++ABI files
  cxa_aux_runtime.cpp
  cxa_default_handlers.cpp
  cxa_demangle.cpp
  cxa_exception_storage.cpp
  cxa_guard.cpp
  cxa_handlers.cpp
  cxa_unexpected.cpp
  cxa_vector.cpp
  cxa_virtual.cpp
  # C++ STL files
  stdlib_exception.cpp
  stdlib_stdexcept.cpp
  stdlib_typeinfo.cpp
  # Internal files
  abort_message.cpp
  fallback_malloc.cpp
  private_typeinfo.cpp
)

if (LIBCXXABI_ENABLE_NEW_DELETE_DEFINITIONS)
  list(APPEND LIBCXXABI_SOURCES stdlib_new_delete.cpp)
endif()

if (LIBCXXABI_ENABLE_EXCEPTIONS)
  list(APPEND LIBCXXABI_SOURCES cxa_exception.cpp)
  list(APPEND LIBCXXABI_SOURCES cxa_personality.cpp)
else()
  list(APPEND LIBCXXABI_SOURCES cxa_noexception.cpp)
endif()

if (LIBCXXABI_ENABLE_THREADS AND (UNIX OR FUCHSIA) AND NOT (APPLE OR CYGWIN))
  list(APPEND LIBCXXABI_SOURCES cxa_thread_atexit.cpp)
endif()

set(LIBCXXABI_HEADERS ../include/cxxabi.h)

# Add all the headers to the project for IDEs.
if (MSVC_IDE OR XCODE)
  # Force them all into the headers dir on MSVC, otherwise they end up at
  # project scope because they don't have extensions.
  if (MSVC_IDE)
    source_group("Header Files" FILES ${LIBCXXABI_HEADERS})
  endif()
endif()

include_directories("${LIBCXXABI_LIBCXX_INCLUDES}")

if (LIBCXXABI_HAS_CXA_THREAD_ATEXIT_IMPL)
  add_definitions(-DHAVE___CXA_THREAD_ATEXIT_IMPL)
endif()

if (LIBCXXABI_ENABLE_THREADS)
  add_library_flags_if(LIBCXXABI_HAS_PTHREAD_LIB pthread)
endif()

add_library_flags_if(LIBCXXABI_HAS_C_LIB c)
if (LIBCXXABI_USE_LLVM_UNWINDER)
  # Prefer using the in-tree version of libunwind, either shared or static. If
  # none are found fall back to using -lunwind.
  # FIXME: Is it correct to prefer the static version of libunwind?
  if (NOT LIBCXXABI_STATICALLY_LINK_UNWINDER_IN_SHARED_LIBRARY AND (TARGET unwind_shared OR HAVE_LIBUNWIND))
    list(APPEND LIBCXXABI_SHARED_LIBRARIES unwind_shared)
  elseif (LIBCXXABI_STATICALLY_LINK_UNWINDER_IN_SHARED_LIBRARY AND (TARGET unwind_static OR HAVE_LIBUNWIND))
    list(APPEND LIBCXXABI_SHARED_LIBRARIES unwind_static)
  else()
    list(APPEND LIBCXXABI_SHARED_LIBRARIES unwind)
  endif()
  if (NOT LIBCXXABI_STATICALLY_LINK_UNWINDER_IN_STATIC_LIBRARY AND (TARGET unwind_shared OR HAVE_LIBUNWIND))
    list(APPEND LIBCXXABI_STATIC_LIBRARIES unwind_shared)
  elseif (LIBCXXABI_STATICALLY_LINK_UNWINDER_IN_STATIC_LIBRARY AND (TARGET unwind_static OR HAVE_LIBUNWIND))
      # We handle this by directly merging libunwind objects into libc++abi.
  else()
    list(APPEND LIBCXXABI_STATIC_LIBRARIES unwind)
  endif()
else()
  add_library_flags_if(LIBCXXABI_HAS_GCC_S_LIB gcc_s)
endif()
if (MINGW)
  # MINGW_LIBRARIES is defined in config-ix.cmake
  list(APPEND LIBCXXABI_LIBRARIES ${MINGW_LIBRARIES})
endif()

# Setup flags.
add_link_flags_if_supported(-nodefaultlibs)

set(LIBCXXABI_SHARED_LINK_FLAGS)

if ( APPLE )
  if ( CMAKE_OSX_DEPLOYMENT_TARGET STREQUAL "10.6" )
    list(APPEND LIBCXXABI_COMPILE_FLAGS "-U__STRICT_ANSI__")
    list(APPEND LIBCXXABI_SHARED_LINK_FLAGS
      "-compatibility_version 1"
      "-current_version 1"
      "-install_name /usr/lib/libc++abi.1.dylib")
    list(APPEND LIBCXXABI_LINK_FLAGS
        "/usr/lib/libSystem.B.dylib")
  else()
    list(APPEND LIBCXXABI_SHARED_LINK_FLAGS
      "-compatibility_version 1"
      "-install_name /usr/lib/libc++abi.1.dylib")
  endif()

  if (LLVM_USE_SANITIZER)
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
      set(LIBCXXABI_SANITIZER_LIBRARY "${LIBDIR}/${LIBFILE}")
      set(LIBCXXABI_SANITIZER_LIBRARY "${LIBCXXABI_SANITIZER_LIBRARY}" PARENT_SCOPE)
      message(STATUS "Manually linking compiler-rt library: ${LIBCXXABI_SANITIZER_LIBRARY}")
      add_library_flags("${LIBCXXABI_SANITIZER_LIBRARY}")
      add_link_flags("-Wl,-rpath,${LIBDIR}")
    endif()
  endif()
endif()

split_list(LIBCXXABI_COMPILE_FLAGS)
split_list(LIBCXXABI_LINK_FLAGS)
split_list(LIBCXXABI_SHARED_LINK_FLAGS)

# FIXME: libc++abi.so will not link when modules are enabled because it depends
# on symbols defined in libc++.so which has not yet been built.
if (LLVM_ENABLE_MODULES)
  string(REPLACE "-Wl,-z,defs" "" CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")
endif()

# Add a object library that contains the compiled source files.
add_library(cxxabi_objects OBJECT ${LIBCXXABI_SOURCES} ${LIBCXXABI_HEADERS})
set_target_properties(cxxabi_objects
                      PROPERTIES
                        CXX_EXTENSIONS
                          OFF
                        CXX_STANDARD
                          11
                        CXX_STANDARD_REQUIRED
                          ON
                        COMPILE_FLAGS
                          "${LIBCXXABI_COMPILE_FLAGS}"
                        POSITION_INDEPENDENT_CODE
                          ON)

# Build the shared library.
if (LIBCXXABI_ENABLE_SHARED)
  add_library(cxxabi_shared SHARED $<TARGET_OBJECTS:cxxabi_objects>)
  if(COMMAND llvm_setup_rpath)
    llvm_setup_rpath(cxxabi_shared)
  endif()
  target_link_libraries(cxxabi_shared ${LIBCXXABI_LIBRARIES} ${LIBCXXABI_SHARED_LIBRARIES})
  set_target_properties(cxxabi_shared
                        PROPERTIES
                          CXX_EXTENSIONS
                            OFF
                          CXX_STANDARD
                            11
                          CXX_STANDARD_REQUIRED
                            ON
                          LINK_FLAGS
                            "${LIBCXXABI_LINK_FLAGS} ${LIBCXXABI_SHARED_LINK_FLAGS}"
                          OUTPUT_NAME
                            "c++abi"
                          POSITION_INDEPENDENT_CODE
                            ON
                          SOVERSION
                            "1"
                          VERSION
                            "1.0")
  list(APPEND LIBCXXABI_BUILD_TARGETS "cxxabi_shared")
  if (LIBCXXABI_INSTALL_SHARED_LIBRARY)
    list(APPEND LIBCXXABI_INSTALL_TARGETS "cxxabi_shared")
  endif()
endif()

# Build the static library.
if (LIBCXXABI_ENABLE_STATIC)
  set(cxxabi_static_sources $<TARGET_OBJECTS:cxxabi_objects>)
  if (LIBCXXABI_USE_LLVM_UNWINDER AND LIBCXXABI_STATICALLY_LINK_UNWINDER_IN_STATIC_LIBRARY)
    if (TARGET unwind_static OR HAVE_LIBUNWIND)
      list(APPEND cxxabi_static_sources $<TARGET_OBJECTS:unwind_objects>)
    endif()
  endif()
  add_library(cxxabi_static STATIC ${cxxabi_static_sources})
  target_link_libraries(cxxabi_static ${LIBCXXABI_LIBRARIES} ${LIBCXXABI_STATIC_LIBRARIES})
  set_target_properties(cxxabi_static
                        PROPERTIES
                          CXX_EXTENSIONS
                            OFF
                          CXX_STANDARD
                            11
                          CXX_STANDARD_REQUIRED
                            ON
                          LINK_FLAGS
                            "${LIBCXXABI_LINK_FLAGS}"
                          OUTPUT_NAME
                            "c++abi"
                          POSITION_INDEPENDENT_CODE
                            ON)
  list(APPEND LIBCXXABI_BUILD_TARGETS "cxxabi_static")
  if (LIBCXXABI_INSTALL_STATIC_LIBRARY)
    list(APPEND LIBCXXABI_INSTALL_TARGETS "cxxabi_static")
  endif()
endif()

# Add a meta-target for both libraries.
add_custom_target(cxxabi DEPENDS ${LIBCXXABI_BUILD_TARGETS})

if (LIBCXXABI_INSTALL_LIBRARY)
  install(TARGETS ${LIBCXXABI_INSTALL_TARGETS}
    LIBRARY DESTINATION ${LIBCXXABI_INSTALL_PREFIX}lib${LIBCXXABI_LIBDIR_SUFFIX} COMPONENT cxxabi
    ARCHIVE DESTINATION ${LIBCXXABI_INSTALL_PREFIX}lib${LIBCXXABI_LIBDIR_SUFFIX} COMPONENT cxxabi
    )
endif()

if (NOT CMAKE_CONFIGURATION_TYPES AND LIBCXXABI_INSTALL_LIBRARY)
  add_custom_target(install-cxxabi
    DEPENDS cxxabi
    COMMAND "${CMAKE_COMMAND}"
            -DCMAKE_INSTALL_COMPONENT=cxxabi
            -P "${LIBCXXABI_BINARY_DIR}/cmake_install.cmake")
  add_custom_target(install-cxxabi-stripped
    DEPENDS cxxabi
    COMMAND "${CMAKE_COMMAND}"
            -DCMAKE_INSTALL_COMPONENT=cxxabi
            -DCMAKE_INSTALL_DO_STRIP=1
            -P "${LIBCXXABI_BINARY_DIR}/cmake_install.cmake")

  # TODO: This is a legacy target name and should be removed at some point.
  add_custom_target(install-libcxxabi DEPENDS install-cxxabi)
endif()
