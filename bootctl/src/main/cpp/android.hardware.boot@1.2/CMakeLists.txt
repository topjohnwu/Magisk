cmake_minimum_required(VERSION 3.18.1)
project(android_hardware_boot-1.2)

set(android_hardware_boot-1.2-sources BootControlAll.cpp)

set(android_hardware_boot-1.2-includes
        include
#        ../libhidl/transport/include
#        ../libhidl/base/include
#        ../android.hidl.base@1.0/include
#        ../libfmq/base
#        ../android.hidl.manager@1.0/include
        )

set(android_hardware_boot-1.2-libraries
        liblog
        libcutils
        libutils
        libhwbinder)

add_library(android_hardware_boot-1.2 SHARED ${android_hardware_boot-1.2-sources})
set_target_properties(android_hardware_boot-1.2 PROPERTIES OUTPUT_NAME android.hardware.boot@1.2)
set_target_properties(android_hardware_boot-1.2 PROPERTIES PREFIX "")

target_include_directories(android_hardware_boot-1.2 PUBLIC ${android_hardware_boot-1.2-includes})
target_link_libraries(android_hardware_boot-1.2 LINK_PUBLIC
#        libhidlbase-static
#        libhidlbase
        ${android_hardware_boot-1.2-libraries}
#        android_hidl_manager-1.1-static
        android_hardware_boot-1.1
        )
