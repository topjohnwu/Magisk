include(CheckLibraryExists)
include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)

if(WIN32 AND NOT MINGW)
  # NOTE(compnerd) this is technically a lie, there is msvcrt, but for now, lets
  # let the default linking take care of that.
  set(LIBCXX_HAS_C_LIB NO)
else()
  check_library_exists(c fopen "" LIBCXX_HAS_C_LIB)
endif()

if (NOT LIBCXX_USE_COMPILER_RT)
  if(WIN32 AND NOT MINGW)
    set(LIBCXX_HAS_GCC_S_LIB NO)
  else()
    check_library_exists(gcc_s __gcc_personality_v0 "" LIBCXX_HAS_GCC_S_LIB)
  endif()
endif()

# libc++ is built with -nodefaultlibs, so we want all our checks to also
# use this option, otherwise we may end up with an inconsistency between
# the flags we think we require during configuration (if the checks are
# performed without -nodefaultlibs) and the flags that are actually
# required during compilation (which has the -nodefaultlibs). libc is
# required for the link to go through. We remove sanitizers from the
# configuration checks to avoid spurious link errors.
check_c_compiler_flag(-nodefaultlibs LIBCXX_SUPPORTS_NODEFAULTLIBS_FLAG)
if (LIBCXX_SUPPORTS_NODEFAULTLIBS_FLAG)
  set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -nodefaultlibs")
  if (LIBCXX_HAS_C_LIB)
    list(APPEND CMAKE_REQUIRED_LIBRARIES c)
  endif ()
  if (LIBCXX_USE_COMPILER_RT)
    list(APPEND CMAKE_REQUIRED_FLAGS -rtlib=compiler-rt)
    find_compiler_rt_library(builtins LIBCXX_BUILTINS_LIBRARY)
    list(APPEND CMAKE_REQUIRED_LIBRARIES "${LIBCXX_BUILTINS_LIBRARY}")
  elseif (LIBCXX_HAS_GCC_S_LIB)
    list(APPEND CMAKE_REQUIRED_LIBRARIES gcc_s)
  endif ()
  if (MINGW)
    # Mingw64 requires quite a few "C" runtime libraries in order for basic
    # programs to link successfully with -nodefaultlibs.
    if (LIBCXX_USE_COMPILER_RT)
      set(MINGW_RUNTIME ${LIBCXX_BUILTINS_LIBRARY})
    else ()
      set(MINGW_RUNTIME gcc_s gcc)
    endif()
    set(MINGW_LIBRARIES mingw32 ${MINGW_RUNTIME} moldname mingwex msvcrt advapi32
                        shell32 user32 kernel32 mingw32 ${MINGW_RUNTIME}
                        moldname mingwex msvcrt)
    list(APPEND CMAKE_REQUIRED_LIBRARIES ${MINGW_LIBRARIES})
  endif()
  if (CMAKE_C_FLAGS MATCHES -fsanitize OR CMAKE_CXX_FLAGS MATCHES -fsanitize)
    set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -fno-sanitize=all")
  endif ()
  if (CMAKE_C_FLAGS MATCHES -fsanitize-coverage OR CMAKE_CXX_FLAGS MATCHES -fsanitize-coverage)
    set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -fno-sanitize-coverage=edge,trace-cmp,indirect-calls,8bit-counters")
  endif ()
endif ()

if(NOT WIN32 OR MINGW)
  include(CheckLibcxxAtomic)
endif()

# Check compiler flags

check_cxx_compiler_flag(/WX                     LIBCXX_HAS_WX_FLAG)
check_cxx_compiler_flag(/WX-                    LIBCXX_HAS_NO_WX_FLAG)
check_cxx_compiler_flag(/EHsc                   LIBCXX_HAS_EHSC_FLAG)
check_cxx_compiler_flag(/EHs-                   LIBCXX_HAS_NO_EHS_FLAG)
check_cxx_compiler_flag(/EHa-                   LIBCXX_HAS_NO_EHA_FLAG)
check_cxx_compiler_flag(/GR-                    LIBCXX_HAS_NO_GR_FLAG)


# Check libraries
if(WIN32 AND NOT MINGW)
  # TODO(compnerd) do we want to support an emulation layer that allows for the
  # use of pthread-win32 or similar libraries to emulate pthreads on Windows?
  set(LIBCXX_HAS_PTHREAD_LIB NO)
  set(LIBCXX_HAS_M_LIB NO)
  set(LIBCXX_HAS_RT_LIB NO)
else()
  check_library_exists(pthread pthread_create "" LIBCXX_HAS_PTHREAD_LIB)
  check_library_exists(m ccos "" LIBCXX_HAS_M_LIB)
  check_library_exists(rt clock_gettime "" LIBCXX_HAS_RT_LIB)
endif()
