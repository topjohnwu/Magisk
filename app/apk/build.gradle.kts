plugins {
    id("com.android.application")
    kotlin("plugin.parcelize")
    kotlin("plugin.compose")
    kotlin("plugin.serialization")
}

setupMainApk()

android {
    buildFeatures {
        compose = true
    }

    compileOptions {
        isCoreLibraryDesugaringEnabled = true
    }

    defaultConfig {
        proguardFile("proguard-rules.pro")
    }

    buildTypes {
        release {
            isMinifyEnabled = true
            isShrinkResources = true
        }
    }
}

dependencies {
    implementation(project(":core"))
    implementation(libs.ui.graphics)
    coreLibraryDesugaring(libs.jdk.libs)

    implementation(libs.appcompat)
    implementation(libs.material)

    // Compose
    implementation(platform(libs.compose.bom))
    implementation(libs.compose.ui)
    implementation(libs.foundation)
    implementation(libs.compose.ui.tooling.preview)
    debugImplementation(libs.compose.ui.tooling)
    implementation(libs.activity.compose)
    implementation(libs.lifecycle.runtime.compose)
    implementation(libs.lifecycle.viewmodel.compose)
    implementation(libs.material.icons.extended)
    implementation(libs.material3)
    implementation(libs.navigation.compose)
    implementation(libs.coil.compose)
    implementation(libs.coil.svg)
    implementation(libs.retrofit)
    implementation(libs.retrofit.moshi)

    // Navigation3
    implementation(libs.navigation3.runtime)
    implementation(libs.navigationevent.compose)
    implementation(libs.lifecycle.viewmodel.navigation3)

    // Terminal
    implementation(libs.termux.terminal.view)
}
