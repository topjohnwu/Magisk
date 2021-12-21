cmake_minimum_required(VERSION 3.18.1)
project(android_hidl_manager-1.2)

set(android_hidl_manager-1.2-sources ClientCallbackAll.cpp ServiceManagerAll.cpp)

set(android_hidl_manager-1.2-includes
        include
        ../libhidl/transport/include
        ../libhidl/base/include
        ../android.hidl.base@1.0/include
        ../libfmq/base)

set(android_hidl_manager-1.2-libraries
        liblog
        libcutils
        libutils
        libhwbinder)

add_library(android_hidl_manager-1.2 SHARED ${android_hidl_manager-1.2-sources})
set_target_properties(android_hidl_manager-1.2 PROPERTIES OUTPUT_NAME android.hidl.manager@1.2)

target_include_directories(android_hidl_manager-1.2 PUBLIC ${android_hidl_manager-1.2-includes})
target_link_libraries(android_hidl_manager-1.2 LINK_PUBLIC ${android_hidl_manager-1.2-libraries}
        libhidlbase-static
        android_hidl_manager-1.0-static)

add_library(android_hidl_manager-1.2-static STATIC ${android_hidl_manager-1.2-sources})
target_include_directories(android_hidl_manager-1.2-static PUBLIC ${android_hidl_manager-1.2-includes})
target_link_libraries(android_hidl_manager-1.2-static LINK_PUBLIC
        ${android_hidl_manager-1.2-libraries}
        libhidlbase-static
        android_hidl_manager-1.0-static)
