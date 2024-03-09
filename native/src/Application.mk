APP_BUILD_SCRIPT := src/Android.mk
APP_ABI          := armeabi-v7a arm64-v8a x86 x86_64
APP_CFLAGS       := -Wall -Oz -fomit-frame-pointer -flto
APP_LDFLAGS      := -flto
APP_CPPFLAGS     := -std=c++20
APP_STL          := none
APP_PLATFORM     := android-23
APP_THIN_ARCHIVE := true
APP_STRIP_MODE   := none

ifdef B_CRT0

# Disable all security and debugging features
APP_CFLAGS       +=	-fno-unwind-tables -fno-asynchronous-unwind-tables -fno-stack-protector -U_FORTIFY_SOURCE
# Override output folder to make sure all dependencies are rebuilt with new CFLAGS
NDK_APP_OUT      := ./obj/nolibc

endif

# Busybox should use stock libc.a
ifdef B_BB
APP_PLATFORM     := android-26
APP_LDFLAGS      += -T src/lto_fix.lds
ifeq ($(OS),Windows_NT)
APP_SHORT_COMMANDS := true
endif
endif
