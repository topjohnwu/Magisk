cmake_minimum_required(VERSION 3.18.1)
project(jsoncpp)

add_library(jsoncpp STATIC
        src/lib_json/json_reader.cpp
        src/lib_json/json_value.cpp
        src/lib_json/json_writer.cpp)
set_target_properties(jsoncpp PROPERTIES PREFIX "")
target_include_directories(jsoncpp PUBLIC include)
target_link_libraries(jsoncpp LINK_PUBLIC libc++)
