cmake_minimum_required(VERSION 3.18.1)
project(libc++)

set(libc++-sources
        src/algorithm.cpp
        src/any.cpp
        src/bind.cpp
        src/charconv.cpp
        src/chrono.cpp
        src/condition_variable.cpp
        src/debug.cpp
        src/exception.cpp
        src/future.cpp
        src/hash.cpp
        src/ios.cpp
        src/iostream.cpp
        src/locale.cpp
        src/memory.cpp
        src/mutex.cpp
        src/new.cpp
        src/optional.cpp
        src/random.cpp
        src/regex.cpp
        src/shared_mutex.cpp
        src/stdexcept.cpp
        src/string.cpp
        src/strstream.cpp
        src/system_error.cpp
        src/thread.cpp
        src/typeinfo.cpp
        src/utility.cpp
        src/valarray.cpp
        src/variant.cpp
        src/vector.cpp)

set(libc++-includes
        include)

set(libc++-libraries
        libc++abi)

set(libc++-definitions
        LIBCXX_BUILDING_LIBCXXABI
        _LIBCPP_BUILDING_LIBRARY
        _LIBCPP_ENABLE_CXX17_REMOVED_UNEXPECTED_FUNCTIONS)

add_library(libc++ SHARED ${libc++-sources})
set_target_properties(libc++ PROPERTIES PREFIX "")
target_include_directories(libc++ PUBLIC ${libc++-includes})
target_link_libraries(libc++ LINK_PUBLIC ${libc++-libraries})
target_compile_definitions(libc++ PRIVATE ${libc++-definitions})

add_library(libc++-static STATIC ${libc++-sources})
target_include_directories(libc++-static PUBLIC ${libc++-includes})
target_link_libraries(libc++-static LINK_PUBLIC ${libc++-libraries})
target_compile_definitions(libc++-static PRIVATE ${libc++-definitions})
