```markdown
## RAFAELIA additions for native/CMakeLists.txt
##
## Purpose:
##  - Detect and link compression & crypto libs for magiskboot.
##  - Support Android NDK multi-ABI builds: prefer NDK-provided libs; fallback to prebuilt static libs in native/prebuilt/<abi>/.
##  - Provide feature-options RAFAELIA_ENABLE_LZ4 / RAFAELIA_ENABLE_LZMA to gate optional features.
##
## Integration instructions:
## 1) Add this block to your existing native/CMakeLists.txt near target definitions for magiskboot.
## 2) If building for Android NDK, provide prebuilt libs under native/prebuilt/<ABI>/
##    Example layout:
##      native/prebuilt/arm64-v8a/liblzma.a
##      native/prebuilt/arm64-v8a/liblz4.a
##      native/prebuilt/arm64-v8a/libcrypto.a  (if needed)
## 3) For host CI runners (Ubuntu), ensure packages installed: zlib1g-dev liblzma-dev liblz4-dev libssl-dev
##
## After editing, rerun cmake and build.

# ---------------- RAFAELIA BEGIN ----------------
# Feature flags
option(RAFAELIA_ENABLE_LZ4 "Enable LZ4 ramdisk support" ON)
option(RAFAELIA_ENABLE_LZMA "Enable XZ ramdisk support" ON)

# Find OpenSSL (libcrypto)
find_package(OpenSSL REQUIRED)

# Find zlib via CMake
find_package(ZLIB REQUIRED)

# Try to find system liblzma and liblz4; if not present and building for Android, fall back to prebuilt under native/prebuilt/<ABI>
if (RAFAELIA_ENABLE_LZMA)
    find_library(LZMA_LIB NAMES lzma)
    if (NOT LZMA_LIB)
        message(STATUS "liblzma not found in system. Will try to use prebuilt if available.")
    endif()
endif()

if (RAFAELIA_ENABLE_LZ4)
    find_library(LZ4_LIB NAMES lz4 lz4frame)
    if (NOT LZ4_LIB)
        message(STATUS "liblz4 not found in system. Will try to use prebuilt if available.")
    endif()
endif()

# Determine if building for Android and choose prebuilt path accordingly
if (ANDROID)
    message(STATUS "Android build detected; ANDROID_ABI=${ANDROID_ABI}")
    set(PREBUILT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/${ANDROID_ABI}")
    if (RAFAELIA_ENABLE_LZMA AND NOT LZMA_LIB)
        set(PREBUILT_LZMA "${PREBUILT_DIR}/liblzma.a")
        if (EXISTS "${PREBUILT_LZMA}")
            message(STATUS "Using prebuilt liblzma for ${ANDROID_ABI}")
            set(LZMA_LIB "${PREBUILT_LZMA}")
        else()
            message(FATAL_ERROR "liblzma missing for ABI ${ANDROID_ABI}. Provide ${PREBUILT_LZMA} or disable RAFAELIA_ENABLE_LZMA.")
        endif()
    endif()

    if (RAFAELIA_ENABLE_LZ4 AND NOT LZ4_LIB)
        set(PREBUILT_LZ4 "${PREBUILT_DIR}/liblz4.a")
        if (EXISTS "${PREBUILT_LZ4}")
            message(STATUS "Using prebuilt liblz4 for ${ANDROID_ABI}")
            set(LZ4_LIB "${PREBUILT_LZ4}")
        else()
            message(FATAL_ERROR "liblz4 missing for ABI ${ANDROID_ABI}. Provide ${PREBUILT_LZ4} or disable RAFAELIA_ENABLE_LZ4.")
        endif()
    endif()
endif()

# Ensure zlib and openssl available
if (NOT ZLIB_FOUND)
    message(FATAL_ERROR "zlib not found; install zlib1g-dev or ensure zlib is available.")
endif()

if (NOT OpenSSL_FOUND)
    message(FATAL_ERROR "OpenSSL not found; install libssl-dev or provide OpenSSL for build.")
endif()

# Link libraries into magiskboot target (assumes target named 'magiskboot' exists)
if (TARGET magiskboot)
    target_link_libraries(magiskboot PRIVATE ZLIB::ZLIB OpenSSL::Crypto)
    if (RAFAELIA_ENABLE_LZMA AND LZMA_LIB)
        target_link_libraries(magiskboot PRIVATE ${LZMA_LIB})
    endif()
    if (RAFAELIA_ENABLE_LZ4 AND LZ4_LIB)
        target_link_libraries(magiskboot PRIVATE ${LZ4_LIB})
    endif()
endif()
# ---------------- RAFAELIA END ----------------
