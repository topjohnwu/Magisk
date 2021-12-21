cmake_minimum_required(VERSION 3.18.1)
project(libcutils)

add_library(libcutils SHARED
        android_get_control_file.cpp
        socket_inaddr_any_server_unix.cpp
        socket_local_client_unix.cpp
        socket_local_server_unix.cpp
        socket_network_client_unix.cpp
        sockets_unix.cpp
        config_utils.cpp
        canned_fs_config.cpp
        iosched_policy.cpp
        load_file.cpp
        native_handle.cpp
        properties.cpp
        record_stream.cpp
        strlcpy.c
        threads.cpp
        fs.cpp
        hashmap.cpp
        multiuser.cpp
        str_parms.cpp
        android_reboot.cpp
        ashmem-dev.cpp
        fs_config.cpp
        klog.cpp
        partition_utils.cpp
        qtaguid.cpp
        trace-dev.cpp
        uevent.cpp)
set_target_properties(libcutils PROPERTIES PREFIX "")
include_directories(include)
target_include_directories(libcutils PUBLIC include)
target_link_libraries(libcutils LINK_PUBLIC liblog libbase)
