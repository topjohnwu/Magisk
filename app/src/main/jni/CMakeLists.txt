cmake_minimum_required(VERSION 3.6)
add_library(zipadjust SHARED
                jni_glue.c
                zipadjust.c)
find_library(libz z)
find_library(liblog log)
target_link_libraries(zipadjust ${libz} ${liblog})
