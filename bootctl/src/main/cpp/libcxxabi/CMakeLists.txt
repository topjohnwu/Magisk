cmake_minimum_required(VERSION 3.18.1)
project(libc++abi)

add_library(libc++abi STATIC
        src/abort_message.cpp
        src/cxa_aux_runtime.cpp
        src/cxa_default_handlers.cpp
        src/cxa_demangle.cpp
        src/cxa_exception.cpp
        src/cxa_exception_storage.cpp
        src/cxa_guard.cpp
        src/cxa_handlers.cpp
        src/cxa_personality.cpp
        src/cxa_thread_atexit.cpp
        src/cxa_unexpected.cpp
        src/cxa_vector.cpp
        src/cxa_virtual.cpp
        src/fallback_malloc.cpp
        src/private_typeinfo.cpp
        src/stdlib_exception.cpp
        src/stdlib_new_delete.cpp
        src/stdlib_stdexcept.cpp
        src/stdlib_typeinfo.cpp)
target_include_directories(libc++abi PUBLIC include ../libcxx/include)
target_link_libraries(libc++abi LINK_PUBLIC libc++-static)
target_compile_definitions(libc++abi PRIVATE
        LIBCXX_BUILDING_LIBCXXABI
        _LIBCPP_BUILDING_LIBRARY
        _LIBCPP_ENABLE_CXX17_REMOVED_UNEXPECTED_FUNCTIONS)
