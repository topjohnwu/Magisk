cmake_minimum_required(VERSION 3.18.1)
project(bootctl)

add_executable(bootctl bootctl.cpp)
target_link_libraries(bootctl LINK_PUBLIC
        android_hardware_boot-1.0
        android_hardware_boot-1.1
        android_hardware_boot-1.2
        libhidlbase
        libutils
        liblog
        libc++
        )
