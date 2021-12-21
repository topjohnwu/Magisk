cmake_minimum_required(VERSION 3.18.1)
project(libbase)

add_library(libbase STATIC
        abi_compatibility.cpp
        chrono_utils.cpp
        cmsg.cpp
        file.cpp
        logging.cpp
        mapped_file.cpp
        parsebool.cpp
        parsenetaddress.cpp
        process.cpp
        properties.cpp
        stringprintf.cpp
        strings.cpp
        threads.cpp
        test_utils.cpp)
target_include_directories(libbase PUBLIC include)
target_link_libraries(libbase LINK_PUBLIC liblog libc++)
