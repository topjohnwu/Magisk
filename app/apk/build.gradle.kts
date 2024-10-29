import com.android.build.api.instrumentation.FramesComputationMode.COMPUTE_FRAMES_FOR_INSTRUMENTED_METHODS
import com.android.build.api.instrumentation.InstrumentationScope

plugins {
    id("com.android.application")
    kotlin("android")
    kotlin("plugin.parcelize")
    kotlin("kapt")
    id("androidx.navigation.safeargs.kotlin")
}

setupAppCommon()

kapt {
    correctErrorTypes = true
    useBuildCache = true
    mapDiagnosticLocations = true
    javacOptions {
        option("-Xmaxerrs", "1000")
    }
}

android {
    namespace = "com.topjohnwu.magisk"

    defaultConfig {
        applicationId = "com.topjohnwu.magisk"
        vectorDrawables.useSupportLibrary = true
        versionName = Config.version
        versionCode = Config.versionCode
        ndk {
            abiFilters += listOf("armeabi-v7a", "arm64-v8a", "x86", "x86_64", "riscv64")
            debugSymbolLevel = "FULL"
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = true
            isShrinkResources = true
            proguardFiles("proguard-rules.pro")
        }
    }

    buildFeatures {
        dataBinding = true
    }

    compileOptions {
        isCoreLibraryDesugaringEnabled = true
    }

    androidComponents {
        onVariants {
            it.instrumentation.setAsmFramesComputationMode(COMPUTE_FRAMES_FOR_INSTRUMENTED_METHODS)
            it.instrumentation.transformClassesWith(
                DesugarClassVisitorFactory::class.java, InstrumentationScope.ALL) {}
        }
    }
}

dependencies {
    implementation(project(":app:core"))
    coreLibraryDesugaring(libs.jdk.libs)

    implementation(libs.indeterminate.checkbox)
    implementation(libs.rikka.layoutinflater)
    implementation(libs.rikka.insets)
    implementation(libs.rikka.recyclerview)

    implementation(libs.navigation.fragment.ktx)
    implementation(libs.navigation.ui.ktx)

    implementation(libs.constraintlayout)
    implementation(libs.swiperefreshlayout)
    implementation(libs.recyclerview)
    implementation(libs.transition)
    implementation(libs.fragment.ktx)
    implementation(libs.appcompat)
    implementation(libs.material)

    // Make sure kapt runs with a proper kotlin-stdlib
    kapt(kotlin("stdlib"))
}
