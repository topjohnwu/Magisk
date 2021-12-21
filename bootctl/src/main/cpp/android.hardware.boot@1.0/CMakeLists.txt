cmake_minimum_required(VERSION 3.18.1)
project(android_hardware_boot-1.0)

set(android_hardware_boot-1.0-sources types.cpp BootControlAll.cpp)

set(android_hardware_boot-1.0-includes
        include
        ../libhidl/transport/include
        ../libhidl/base/include
        ../android.hidl.base@1.0/include
        ../libfmq/base
        ../android.hidl.manager@1.0/include)

set(android_hardware_boot-1.0-libraries
        liblog
        libcutils
        libutils
        libhwbinder)

add_library(android_hardware_boot-1.0 SHARED ${android_hardware_boot-1.0-sources})
set_target_properties(android_hardware_boot-1.0 PROPERTIES OUTPUT_NAME android.hardware.boot@1.0)
set_target_properties(android_hardware_boot-1.0 PROPERTIES PREFIX "")

target_include_directories(android_hardware_boot-1.0 PUBLIC ${android_hardware_boot-1.0-includes})
target_link_libraries(android_hardware_boot-1.0 LINK_PUBLIC ${android_hardware_boot-1.0-libraries} libhidlbase-static android_hidl_manager-1.1-static)

add_library(android_hardware_boot-1.0-static STATIC ${android_hardware_boot-1.0-sources})
target_include_directories(android_hardware_boot-1.0-static PUBLIC ${android_hardware_boot-1.0-includes})
target_link_libraries(android_hardware_boot-1.0-static LINK_PUBLIC ${android_hardware_boot-1.0-libraries} libhidlbase-static android_hidl_manager-1.1-static)

