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

afterEvaluate {
    val adb = androidComponents.sdkComponents.adb.get().asFile.absolutePath

    val pushTask = task("pushEmulator", Exec::class) {
        commandLine(adb, "push", "native/out/x86_64/busybox", "out/app-debug.apk", "scripts/emulator.sh", "/data/local/tmp")
        workingDir(rootDir)
    }
    task("setupEmulator", Exec::class) {
        dependsOn(pushTask)
        commandLine(adb, "shell", "sh", "/data/local/tmp/emulator.sh")
    }
}
