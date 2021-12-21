cmake_minimum_required(VERSION 3.18.1)
project(libhwbinder)
add_library(libhwbinder STATIC
        Binder.cpp
        BpHwBinder.cpp
        BufferedTextOutput.cpp
        Debug.cpp
        IInterface.cpp
        IPCThreadState.cpp
        Parcel.cpp
        ProcessState.cpp
        Static.cpp
        TextOutput.cpp
        Utils.cpp)
target_include_directories(libhwbinder PUBLIC include)
target_link_libraries(libhwbinder LINK_PUBLIC libutils libcutils)
