APP_ABI          := armeabi-v7a arm64-v8a x86 x86_64
APP_CFLAGS       := -Wall -Oz -fomit-frame-pointer -flto
APP_LDFLAGS      := -flto
APP_CPPFLAGS     := -std=c++20
APP_STL          := none
APP_PLATFORM     := android-21
APP_THIN_ARCHIVE := true
APP_STRIP_MODE   := --strip-all

ifndef B_SHARED
# Fix static variables' ctor/dtor when using LTO
# See: https://github.com/android/ndk/issues/1461
APP_LDFLAGS      += -T jni/lto_fix.lds
endif

# Busybox should use stock libc.a
ifdef B_BB
APP_PLATFORM     := android-24
ifeq ($(OS),Windows_NT)
APP_SHORT_COMMANDS := true
endif
endif
