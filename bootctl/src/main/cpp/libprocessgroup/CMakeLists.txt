cmake_minimum_required(VERSION 3.18.1)
project(libprocessgroup)

add_library(libprocessgroup STATIC
        cgroup_map.cpp
        processgroup.cpp
        sched_policy.cpp
        task_profiles.cpp
        cgrouprc/cgroup_controller.cpp
        cgrouprc/cgroup_file.cpp
        cgrouprc_format/cgroup_controller.cpp)
set_target_properties(libprocessgroup PROPERTIES PREFIX "")
target_include_directories(libprocessgroup PRIVATE . include cgrouprc/include cgrouprc_format/include PUBLIC include)
target_link_libraries(libprocessgroup LINK_PUBLIC libbase libcutils jsoncpp)
