cmake_minimum_required(VERSION 3.18.1)
project(liblog)
add_library(liblog STATIC
        log_event_list.cpp
        log_event_write.cpp
        logger_name.cpp
        logger_read.cpp
        logger_write.cpp
        logprint.cpp
        properties.cpp
        event_tag_map.cpp
        log_time.cpp
        pmsg_reader.cpp
        pmsg_writer.cpp
        logd_reader.cpp
        logd_writer.cpp)
target_include_directories(liblog PUBLIC include)
target_link_libraries(liblog LINK_PUBLIC libsystem libc++)
include_directories(../libutils/include ../libcutils/include ../libbase/include)
target_compile_definitions(liblog PRIVATE LIBLOG_LOG_TAG=1006 SNET_EVENT_LOG_TAG=1397638484)
