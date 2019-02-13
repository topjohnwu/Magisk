APP_ABI := armeabi-v7a x86
APP_CFLAGS := -Oz -std=gnu11 \
    -fno-exceptions -fno-rtti -fomit-frame-pointer \
    -D__MVSTR=${MAGISK_VERSION} -D__MCODE=${MAGISK_VER_CODE}
APP_CPPFLAGS := -std=c++17
APP_STL := c++_static
APP_PLATFORM := android-16

ifdef MAGISK_DEBUG
APP_CFLAGS += -D__MDBG
endif

# Busybox require some additional settings
ifdef B_BB
APP_CFLAGS := -Os
APP_SHORT_COMMANDS := true
NDK_TOOLCHAIN_VERSION := 4.9
APP_PLATFORM := android-22
endif
