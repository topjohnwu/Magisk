cmake_minimum_required(VERSION 3.18.1)
project(libvndksupport)

add_library(libvndksupport STATIC linker.cpp)
set_target_properties(libvndksupport PROPERTIES PREFIX "")
include_directories(include/vndksupport)
target_include_directories(libvndksupport PUBLIC include)
target_link_libraries(libvndksupport LINK_PUBLIC liblog libdl)
