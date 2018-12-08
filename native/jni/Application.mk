APP_ABI := armeabi-v7a x86
APP_CFLAGS := -Oz -std=gnu11 \
-DMAGISK_VERSION="${MAGISK_VERSION}" -DMAGISK_VER_CODE=${MAGISK_VER_CODE}
APP_CPPFLAGS := -std=c++14
APP_STL := system
APP_PLATFORM := android-16

ifdef MAGISK_DEBUG
APP_CFLAGS += -DMAGISK_DEBUG
endif

# Busybox require some additional settings
ifdef B_BB
APP_CFLAGS := -Os
APP_SHORT_COMMANDS := true
NDK_TOOLCHAIN_VERSION := 4.9
APP_PLATFORM := android-22
endif
