cmake_minimum_required(VERSION 3.18.1)
project(libdl)

add_library(libdl SHARED
        libdl_android.cpp
        libdl.cpp)
set_target_properties(libdl PROPERTIES PREFIX "")
target_include_directories(libdl PUBLIC include)
