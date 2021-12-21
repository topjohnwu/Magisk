cmake_minimum_required(VERSION 3.18.1)
project(libhidl)

set(libhidlbase-sources
        base/HidlInternal.cpp
        base/HidlSupport.cpp
        base/Status.cpp
        base/TaskRunner.cpp
        transport/HidlBinderSupport.cpp
        transport/HidlLazyUtils.cpp
        transport/HidlPassthroughSupport.cpp
        transport/HidlTransportSupport.cpp
        transport/HidlTransportUtils.cpp
        transport/LegacySupport.cpp
        transport/ServiceManagement.cpp
        transport/Static.cpp)

set(libhidlbase-includes
        base/include
        transport/include
        ../android.hidl.manager@1.0/include
        ../android.hidl.manager@1.1/include
        ../android.hidl.manager@1.2/include
        ../libfmq/base)

set(libhidlbase-libraries
        liblog
        libcutils
        libutils
        libhwbinder
        libc++)

add_library(libhidlbase SHARED ${libhidlbase-sources})
set_target_properties(libhidlbase PROPERTIES PREFIX "")
target_include_directories(libhidlbase PUBLIC ${libhidlbase-includes})
target_link_libraries(libhidlbase LINK_PUBLIC ${libhidlbase-libraries} android_hidl_base-1.0-static android_hidl_manager-1.0-static)

add_library(libhidlbase-static STATIC ${libhidlbase-sources})
target_include_directories(libhidlbase-static PUBLIC ${libhidlbase-includes})
target_link_libraries(libhidlbase-static LINK_PUBLIC ${libhidlbase-libraries} android_hidl_base-1.0-static android_hidl_manager-1.0-static)