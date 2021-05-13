APP_ABI := armeabi-v7a arm64-v8a x86 x86_64
APP_CFLAGS := -Wall -Oz -fomit-frame-pointer -flto \
-D__MVSTR=${MAGISK_VERSION} -D__MCODE=${MAGISK_VER_CODE}
APP_LDFLAGS := -flto
APP_CPPFLAGS := -std=c++17
APP_STL := none
APP_PLATFORM := android-16

ifdef MAGISK_DEBUG
APP_CFLAGS += -D__MDBG
endif

# Busybox should use stock libc.a
ifdef B_BB
APP_PLATFORM := android-22
endif
