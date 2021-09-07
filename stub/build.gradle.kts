import org.gradle.internal.os.OperatingSystem
import java.io.OutputStream
import java.io.PrintStream
import java.nio.file.Paths

plugins {
    id("com.android.application")
    id("io.michaelrocks.paranoid")
}

paranoid {
    obfuscationSeed = if (RAND_SEED != 0) RAND_SEED else null
    includeSubprojects = true
}

setupApp()

android {
    val canary = !Config.version.contains(".")

    val url = Config["DEV_CHANNEL"] ?: if (canary) null
    else "https://cdn.jsdelivr.net/gh/topjohnwu/magisk-files@${Config.version}/app-release.apk"

    defaultConfig {
        applicationId = "com.topjohnwu.magisk"
        versionCode = 1
        versionName = "1.0"
        buildConfigField("int", "STUB_VERSION", Config.stubVersion)
        buildConfigField("String", "APK_URL", url?.let { "\"$it\"" } ?: "null" )
    }

    buildTypes {
        release {
            isMinifyEnabled = true
            isShrinkResources = false
            proguardFiles("proguard-rules.pro")
        }
    }

    dependenciesInfo {
        includeInApk = false
        includeInBundle = false
    }
}

// Make sure we have a working manifest while building
val ensureManifest by tasks.registering {
    val manifest = file("src/main/AndroidManifest.xml")
    if (!manifest.exists()) {
        PrintStream(manifest).use {
            it.println("<manifest package=\"com.topjohnwu.magisk\"/>")
        }
    }
}

tasks.preBuild {
    dependsOn(ensureManifest)
}

android.applicationVariants.all {
    val manifest = file("src/main/AndroidManifest.xml")
    val outSrcDir = File(buildDir, "generated/source/obfuscate/$name")
    val templateDir = file("template")
    val resDir = file("res")

    val androidJar = Paths.get(android.sdkDirectory.path, "platforms",
        android.compileSdkVersion, "android.jar")

    val aaptCommand = if (OperatingSystem.current().isWindows) "aapt2.exe" else "aapt2"
    val aapt = Paths.get(android.sdkDirectory.path,
        "build-tools", android.buildToolsVersion, aaptCommand)

    val dummy = object : OutputStream() {
        override fun write(b: Int) {}
        override fun write(bytes: ByteArray, off: Int, len: Int) {}
    }

    val genSrcTask = tasks.register("generate${name.capitalize()}ObfuscatedSources") {
        doLast {
            val xml = genStubManifest(templateDir, outSrcDir)
            PrintStream(manifest).use {
                it.print(xml)
            }

            val compileTmp = File.createTempFile("tmp", ".zip")
            val linkTmp = File.createTempFile("tmp", ".zip")
            val optTmp = File.createTempFile("tmp", ".zip")
            val stubXml = File.createTempFile("tmp", ".xml")
            try {
                PrintStream(stubXml).use {
                    it.println("<manifest package=\"com.topjohnwu.magisk\"/>")
                }

                exec {
                    commandLine(aapt, "compile",
                        "-o", compileTmp,
                        "--dir", resDir)
                    standardOutput = dummy
                    errorOutput = dummy
                }

                exec {
                    commandLine(aapt, "link",
                        "-o", linkTmp,
                        "-I", androidJar,
                        "--min-sdk-version", android.defaultConfig.minSdk,
                        "--target-sdk-version", android.defaultConfig.targetSdk,
                        "--manifest", stubXml,
                        "--java", outSrcDir, compileTmp)
                    standardOutput = dummy
                    errorOutput = dummy
                }

                exec {
                    commandLine(aapt, "optimize",
                        "-o", optTmp,
                        "--collapse-resource-names", linkTmp)
                    standardOutput = dummy
                    errorOutput = dummy
                }

                genEncryptedResources(optTmp, outSrcDir)
            } finally {
                compileTmp.delete()
                linkTmp.delete()
                optTmp.delete()
                stubXml.delete()
            }
        }
    }
    registerJavaGeneratingTask(genSrcTask, outSrcDir)
}

dependencies {
    implementation(project(":app:shared"))
}
