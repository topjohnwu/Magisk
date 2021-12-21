INCLUDE(CheckCXXSourceCompiles)

# Sometimes linking against libatomic is required for atomic ops, if
# the platform doesn't support lock-free atomics.
#
# We could modify LLVM's CheckAtomic module and have it check for 64-bit
# atomics instead. However, we would like to avoid careless uses of 64-bit
# atomics inside LLVM over time on 32-bit platforms.

function(check_cxx_atomics varname)
  set(OLD_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
  set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -nodefaultlibs -std=c++11 -nostdinc++ -isystem ${LIBCXX_SOURCE_DIR}/include")
  if (${LIBCXX_GCC_TOOLCHAIN})
    set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} --gcc-toolchain=${LIBCXX_GCC_TOOLCHAIN}")
  endif()
  if (CMAKE_C_FLAGS MATCHES -fsanitize OR CMAKE_CXX_FLAGS MATCHES -fsanitize)
    set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -fno-sanitize=all")
  endif()
  if (CMAKE_C_FLAGS MATCHES -fsanitize-coverage OR CMAKE_CXX_FLAGS MATCHES -fsanitize-coverage)
    set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -fno-sanitize-coverage=edge,trace-cmp,indirect-calls,8bit-counters")
  endif()
  check_cxx_source_compiles("
#include <cstdint>
#include <atomic>
std::atomic<uintptr_t> x;
std::atomic<uintmax_t> y;
int main() {
  return x + y;
}
" ${varname})
  set(CMAKE_REQUIRED_FLAGS ${OLD_CMAKE_REQUIRED_FLAGS})
endfunction(check_cxx_atomics)

# Perform the check for 64bit atomics without libatomic. It may have been
# added to the required libraries during in the configuration of LLVM, which
# would cause the check for CXX atomics without libatomic to incorrectly pass.
set(OLD_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
list(REMOVE_ITEM CMAKE_REQUIRED_LIBRARIES "atomic")
check_cxx_atomics(LIBCXX_HAVE_CXX_ATOMICS_WITHOUT_LIB)
set(CMAKE_REQUIRED_LIBRARIES ${OLD_CMAKE_REQUIRED_LIBRARIES})

check_library_exists(atomic __atomic_fetch_add_8 "" LIBCXX_HAS_ATOMIC_LIB)
# If not, check if the library exists, and atomics work with it.
if(NOT LIBCXX_HAVE_CXX_ATOMICS_WITHOUT_LIB)
  if(LIBCXX_HAS_ATOMIC_LIB)
    list(APPEND CMAKE_REQUIRED_LIBRARIES "atomic")
    check_cxx_atomics(LIBCXX_HAVE_CXX_ATOMICS_WITH_LIB)
    if (NOT LIBCXX_HAVE_CXX_ATOMICS_WITH_LIB)
      message(WARNING "Host compiler must support std::atomic!")
    endif()
  else()
    message(WARNING "Host compiler appears to require libatomic, but cannot find it.")
  endif()
endif()
