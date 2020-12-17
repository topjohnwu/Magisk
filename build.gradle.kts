import com.android.build.gradle.BaseExtension
import java.nio.file.Paths

plugins {
    id("MagiskPlugin")
}

// Top-level build file where you can add configuration options common to all sub-projects/modules.

buildscript {
    repositories {
        google()
        jcenter()
        maven { url = uri("https://kotlin.bintray.com/kotlinx") }
    }

    val vNav = "2.3.4"
    extra["vNav"] = vNav

    dependencies {
        classpath("com.android.tools.build:gradle:4.1.3")
        classpath("org.jetbrains.kotlin:kotlin-gradle-plugin:1.4.31")
        classpath("androidx.navigation:navigation-safe-args-gradle-plugin:${vNav}")

        // NOTE: Do not place your application dependencies here; they belong
        // in the individual module build.gradle files
    }
}

tasks.register("clean", Delete::class) {
    delete(rootProject.buildDir)
}

val Project.android get() = extensions.getByName<BaseExtension>("android")

fun Task.applyOptimize() = doLast {
    val aapt2 = Paths.get(project.android.sdkDirectory.path,
        "build-tools", project.android.buildToolsVersion, "aapt2")
    val zip = Paths.get(project.buildDir.path, "intermediates",
        "shrunk_processed_res", "release", "resources-release-stripped.ap_")
    val optimized = File("${zip}.opt")
    val cmd = exec {
        commandLine(aapt2, "optimize", "--collapse-resource-names",
            "--shorten-resource-paths", "-o", optimized, zip)
        isIgnoreExitValue = true
    }
    if (cmd.exitValue == 0) {
        zip.toFile().delete()
        optimized.renameTo(zip.toFile())
    }
}

subprojects {
    repositories {
        google()
        jcenter()
        maven { url = uri("https://jitpack.io") }
        maven { url = uri("http://oss.sonatype.org/content/repositories/snapshots") }
    }

    afterEvaluate {
        if (plugins.hasPlugin("com.android.library") ||
            plugins.hasPlugin("com.android.application")) {
            android.apply {
                compileSdkVersion(30)
                buildToolsVersion = "30.0.3"
                ndkPath = "${System.getenv("ANDROID_SDK_ROOT")}/ndk/magisk"

                defaultConfig {
                    if (minSdkVersion == null)
                        minSdkVersion(17)
                    targetSdkVersion(30)
                }

                compileOptions {
                    sourceCompatibility = JavaVersion.VERSION_1_8
                    targetCompatibility = JavaVersion.VERSION_1_8
                }
            }
        }

        if (plugins.hasPlugin("java")) {
            tasks.withType<JavaCompile> {
                // If building with JDK 9+, we need additional flags to generate compatible bytecode
                if (JavaVersion.current() > JavaVersion.VERSION_1_8) {
                    options.compilerArgs.addAll(listOf("--release", "8"))
                }
            }
        }

        tasks.whenTaskAdded {
            if (name == "shrinkReleaseRes") {
                finalizedBy(tasks.create("optimizeReleaseRes").applyOptimize())
            }
        }

        if (name == "app" || name == "stub") {
            android.apply {
                signingConfigs {
                    create("config") {
                        Config["keyStore"]?.also {
                            storeFile = rootProject.file(it)
                            storePassword = Config["keyStorePass"]
                            keyAlias = Config["keyAlias"]
                            keyPassword = Config["keyPass"]
                        }
                    }
                }

                buildTypes {
                    signingConfigs.getByName("config").also {
                        getByName("debug") {
                            signingConfig = if (it.storeFile?.exists() == true) it
                            else signingConfigs.getByName("debug")
                        }
                        getByName("release") {
                            signingConfig = if (it.storeFile?.exists() == true) it
                            else signingConfigs.getByName("debug")
                        }
                    }
                }

                lintOptions {
                    disable("MissingTranslation")
                }
            }
        }
    }
}
