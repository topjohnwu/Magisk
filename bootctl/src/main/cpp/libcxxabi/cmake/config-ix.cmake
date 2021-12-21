include(CheckLibraryExists)
include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)

check_library_exists(c fopen "" LIBCXXABI_HAS_C_LIB)
if (NOT LIBCXXABI_USE_COMPILER_RT)
  check_library_exists(gcc_s __gcc_personality_v0 "" LIBCXXABI_HAS_GCC_S_LIB)
endif ()

# libc++abi is built with -nodefaultlibs, so we want all our checks to also
# use this option, otherwise we may end up with an inconsistency between
# the flags we think we require during configuration (if the checks are
# performed without -nodefaultlibs) and the flags that are actually
# required during compilation (which has the -nodefaultlibs). libc is
# required for the link to go through. We remove sanitizers from the
# configuration checks to avoid spurious link errors.
check_c_compiler_flag(-nodefaultlibs LIBCXXABI_HAS_NODEFAULTLIBS_FLAG)
if (LIBCXXABI_HAS_NODEFAULTLIBS_FLAG)
  set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -nodefaultlibs")
  if (LIBCXXABI_HAS_C_LIB)
    list(APPEND CMAKE_REQUIRED_LIBRARIES c)
  endif ()
  if (LIBCXXABI_USE_COMPILER_RT)
    list(APPEND CMAKE_REQUIRED_FLAGS -rtlib=compiler-rt)
    find_compiler_rt_library(builtins LIBCXXABI_BUILTINS_LIBRARY)
    list(APPEND CMAKE_REQUIRED_LIBRARIES "${LIBCXXABI_BUILTINS_LIBRARY}")
  elseif (LIBCXXABI_HAS_GCC_S_LIB)
    list(APPEND CMAKE_REQUIRED_LIBRARIES gcc_s)
  endif ()
  if (MINGW)
    # Mingw64 requires quite a few "C" runtime libraries in order for basic
    # programs to link successfully with -nodefaultlibs.
    if (LIBCXXABI_USE_COMPILER_RT)
      set(MINGW_RUNTIME ${LIBCXXABI_BUILTINS_LIBRARY})
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

# Check compiler flags
check_c_compiler_flag(-funwind-tables         LIBCXXABI_HAS_FUNWIND_TABLES)
check_cxx_compiler_flag(-fno-exceptions       LIBCXXABI_HAS_NO_EXCEPTIONS_FLAG)
check_cxx_compiler_flag(-fno-rtti             LIBCXXABI_HAS_NO_RTTI_FLAG)
check_cxx_compiler_flag(-fstrict-aliasing     LIBCXXABI_HAS_FSTRICT_ALIASING_FLAG)
check_cxx_compiler_flag(-nostdinc++           LIBCXXABI_HAS_NOSTDINCXX_FLAG)
check_cxx_compiler_flag(-Wall                 LIBCXXABI_HAS_WALL_FLAG)
check_cxx_compiler_flag(-W                    LIBCXXABI_HAS_W_FLAG)
check_cxx_compiler_flag(-Wunused-function     LIBCXXABI_HAS_WUNUSED_FUNCTION_FLAG)
check_cxx_compiler_flag(-Wunused-variable     LIBCXXABI_HAS_WUNUSED_VARIABLE_FLAG)
check_cxx_compiler_flag(-Wunused-parameter    LIBCXXABI_HAS_WUNUSED_PARAMETER_FLAG)
check_cxx_compiler_flag(-Wstrict-aliasing     LIBCXXABI_HAS_WSTRICT_ALIASING_FLAG)
check_cxx_compiler_flag(-Wstrict-overflow     LIBCXXABI_HAS_WSTRICT_OVERFLOW_FLAG)
check_cxx_compiler_flag(-Wwrite-strings       LIBCXXABI_HAS_WWRITE_STRINGS_FLAG)
check_cxx_compiler_flag(-Wchar-subscripts     LIBCXXABI_HAS_WCHAR_SUBSCRIPTS_FLAG)
check_cxx_compiler_flag(-Wmismatched-tags     LIBCXXABI_HAS_WMISMATCHED_TAGS_FLAG)
check_cxx_compiler_flag(-Wmissing-braces      LIBCXXABI_HAS_WMISSING_BRACES_FLAG)
check_cxx_compiler_flag(-Wshorten-64-to-32    LIBCXXABI_HAS_WSHORTEN_64_TO_32_FLAG)
check_cxx_compiler_flag(-Wsign-conversion     LIBCXXABI_HAS_WSIGN_CONVERSION_FLAG)
check_cxx_compiler_flag(-Wsign-compare        LIBCXXABI_HAS_WSIGN_COMPARE_FLAG)
check_cxx_compiler_flag(-Wshadow              LIBCXXABI_HAS_WSHADOW_FLAG)
check_cxx_compiler_flag(-Wconversion          LIBCXXABI_HAS_WCONVERSION_FLAG)
check_cxx_compiler_flag(-Wnewline-eof         LIBCXXABI_HAS_WNEWLINE_EOF_FLAG)
check_cxx_compiler_flag(-Wundef               LIBCXXABI_HAS_WUNDEF_FLAG)
check_cxx_compiler_flag(-pedantic             LIBCXXABI_HAS_PEDANTIC_FLAG)
check_cxx_compiler_flag(-Werror               LIBCXXABI_HAS_WERROR_FLAG)
check_cxx_compiler_flag(-Wno-error            LIBCXXABI_HAS_WNO_ERROR_FLAG)
check_cxx_compiler_flag(/WX                   LIBCXXABI_HAS_WX_FLAG)
check_cxx_compiler_flag(/WX-                  LIBCXXABI_HAS_NO_WX_FLAG)
check_cxx_compiler_flag(/EHsc                 LIBCXXABI_HAS_EHSC_FLAG)
check_cxx_compiler_flag(/EHs-                 LIBCXXABI_HAS_NO_EHS_FLAG)
check_cxx_compiler_flag(/EHa-                 LIBCXXABI_HAS_NO_EHA_FLAG)
check_cxx_compiler_flag(/GR-                  LIBCXXABI_HAS_NO_GR_FLAG)

# Check libraries
check_library_exists(dl dladdr "" LIBCXXABI_HAS_DL_LIB)
check_library_exists(pthread pthread_once "" LIBCXXABI_HAS_PTHREAD_LIB)
check_library_exists(c __cxa_thread_atexit_impl ""
  LIBCXXABI_HAS_CXA_THREAD_ATEXIT_IMPL)
