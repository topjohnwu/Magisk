import java.io.ByteArrayOutputStream

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

    task("setupEmulator", Exec::class) {
        doFirst {
            val abi = ByteArrayOutputStream().use { outputStream ->
                exec {
                    commandLine(adb, "shell", "getprop", "ro.product.cpu.abi")
                    standardOutput = outputStream
                }
                print("abi is $outputStream")
                outputStream.toString().trim()
            }
            exec {
                commandLine(adb, "push", "native/out/$abi/busybox", "out/app-debug.apk", "scripts/emulator.sh", "/data/local/tmp")
                workingDir(rootDir)
            }
        }
        commandLine(adb, "shell", "sh", "/data/local/tmp/emulator.sh")
    }
}
