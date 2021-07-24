import com.android.build.gradle.BaseExtension

plugins {
    id("MagiskPlugin")
}

// Top-level build file where you can add configuration options common to all sub-projects/modules.

buildscript {
    repositories {
        google()
        mavenCentral()
    }

    val vNav = "2.3.5"
    extra["vNav"] = vNav

    dependencies {
        classpath("com.android.tools.build:gradle:7.0.0")
        classpath(kotlin("gradle-plugin", version = "1.5.21"))
        classpath("androidx.navigation:navigation-safe-args-gradle-plugin:${vNav}")

        // NOTE: Do not place your application dependencies here; they belong
        // in the individual module build.gradle files
    }
}

tasks.register("clean", Delete::class) {
    delete(rootProject.buildDir)
}

fun Project.android(configuration: BaseExtension.() -> Unit) =
    extensions.getByName<BaseExtension>("android").configuration()

subprojects {
    repositories {
        google()
        mavenCentral()
        maven { url = uri("https://jitpack.io") }
    }

    afterEvaluate {
        if (plugins.hasPlugin("com.android.library") ||
            plugins.hasPlugin("com.android.application")) {
            android {
                compileSdkVersion(30)
                buildToolsVersion = "30.0.3"
                ndkPath = "${System.getenv("ANDROID_SDK_ROOT")}/ndk/magisk"

                defaultConfig {
                    if (minSdk == null)
                        minSdk = 21
                    targetSdk = 30
                }

                compileOptions {
                    sourceCompatibility = JavaVersion.VERSION_11
                    targetCompatibility = JavaVersion.VERSION_11
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

        if (name == "app" || name == "stub") {
            android {
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
