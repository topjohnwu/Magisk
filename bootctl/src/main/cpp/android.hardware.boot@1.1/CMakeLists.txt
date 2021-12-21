cmake_minimum_required(VERSION 3.18.1)
project(android_hardware_boot-1.1)

set(android_hardware_boot-1.1-sources types.cpp BootControlAll.cpp)

set(android_hardware_boot-1.1-includes
        include
#        ../libhidl/transport/include
#        ../libhidl/base/include
        ../android.hidl.base@1.0/include
#        ../libfmq/base
        ../android.hidl.manager@1.0/include)

set(android_hardware_boot-1.1-libraries
        liblog
        libcutils
        libutils
        libhwbinder)

add_library(android_hardware_boot-1.1 SHARED ${android_hardware_boot-1.1-sources})
set_target_properties(android_hardware_boot-1.1 PROPERTIES OUTPUT_NAME android.hardware.boot@1.1)
set_target_properties(android_hardware_boot-1.1 PROPERTIES PREFIX "")

target_include_directories(android_hardware_boot-1.1 PUBLIC ${android_hardware_boot-1.1-includes})
target_link_libraries(android_hardware_boot-1.1 LINK_PUBLIC
        libhidlbase
#        libhidlbase-static
#        android_hidl_manager-1.1-static
        ${android_hardware_boot-1.1-libraries}
        )

#add_library(android_hardware_boot-1.1-static STATIC ${android_hardware_boot-1.1-sources})
#target_include_directories(android_hardware_boot-1.1-static PUBLIC ${android_hardware_boot-1.1-includes})
#target_link_libraries(android_hardware_boot-1.1-static LINK_PUBLIC ${android_hardware_boot-1.1-libraries} libhidlbase-static android_hidl_manager-1.1-static)

