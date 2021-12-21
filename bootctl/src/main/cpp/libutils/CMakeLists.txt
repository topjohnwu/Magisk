cmake_minimum_required(VERSION 3.18.1)
project(libutils)

add_library(libutils SHARED
        Errors.cpp
        FileMap.cpp
        JenkinsHash.cpp
        LightRefBase.cpp
        NativeHandle.cpp
        Printer.cpp
        RefBase.cpp
        SharedBuffer.cpp
        StopWatch.cpp
        String8.cpp
        String16.cpp
        StrongPointer.cpp
        SystemClock.cpp
        Threads.cpp
        Timers.cpp
        Tokenizer.cpp
        Unicode.cpp
        VectorImpl.cpp
        misc.cpp)
set_target_properties(libutils PROPERTIES PREFIX "")
target_include_directories(libutils PUBLIC include ../libbacktrace/include ../libprocessgroup/include)
#include_directories(../liblog/include)
target_link_libraries(libutils LINK_PUBLIC libbase liblog libsystem libcutils libvndksupport libprocessgroup)
