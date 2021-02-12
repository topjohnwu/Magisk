plugins {
    id("com.android.library")
}

android {

    externalNativeBuild {
        ndkBuild {
            path("jni/Android.mk")
        }
    }

    defaultConfig {
        externalNativeBuild {
            ndkBuild {
                // Pass arguments to ndk-build.
                arguments("B_MAGISK=1", "B_INIT=1", "B_BOOT=1", "B_TEST=1",
                        "MAGISK_DEBUG=1", "MAGISK_VERSION=debug", "MAGISK_VER_CODE=INT_MAX")
            }
        }
    }
}
