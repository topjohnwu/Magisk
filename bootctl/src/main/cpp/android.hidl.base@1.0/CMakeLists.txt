cmake_minimum_required(VERSION 3.18.1)
project(android_hidl_base-1.0)

set(android_hidl_base-1.0-sources types.cpp BaseAll.cpp)

set(android_hidl_base-1.0-includes
        include
        ../libhidl/transport/include
        ../libhidl/base/include
        ../libfmq/base)

set(android_hidl_base-1.0-libraries
        liblog
        libcutils
        libutils
        libhwbinder)

add_library(android_hidl_base-1.0 SHARED ${android_hidl_base-1.0-sources})
set_target_properties(android_hidl_base-1.0 PROPERTIES OUTPUT_NAME android.hidl.base@1.0)

target_include_directories(android_hidl_base-1.0 PUBLIC ${android_hidl_base-1.0-includes})
target_link_libraries(android_hidl_base-1.0 LINK_PUBLIC ${android_hidl_base-1.0-libraries} android_hidl_manager-1.0-static)
target_compile_options(android_hidl_base-1.0 PUBLIC -Wno-inconsistent-missing-override)
#target_compile_features(android_hidl_base-1.0 PUBLIC c_std_99)

add_library(android_hidl_base-1.0-static STATIC ${android_hidl_base-1.0-sources})
target_include_directories(android_hidl_base-1.0-static PUBLIC ${android_hidl_base-1.0-includes})
target_link_libraries(android_hidl_base-1.0-static LINK_PUBLIC ${android_hidl_base-1.0-libraries} android_hidl_manager-1.0-static)
target_compile_options(android_hidl_base-1.0-static PUBLIC -Wno-inconsistent-missing-override)
#target_compile_features(android_hidl_base-1.0-static PUBLIC c_std_99)
