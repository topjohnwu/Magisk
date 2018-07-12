APP_ABI := x86 armeabi-v7a
APP_CFLAGS := -std=gnu99 ${MAGISK_DEBUG} \
	-DMAGISK_VERSION="${MAGISK_VERSION}" -DMAGISK_VER_CODE=${MAGISK_VER_CODE} 
APP_CPPFLAGS := -std=c++11
APP_SHORT_COMMANDS := true
ifdef OLD_PLAT
APP_PLATFORM := android-16
APP_CFLAGS += -Wno-implicit-function-declaration
else
APP_PLATFORM := android-21	
endif
