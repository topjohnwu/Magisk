plugins {
    alias(libs.plugins.android.library)
    kotlin("plugin.parcelize")
    alias(libs.plugins.compose.compiler)
}

setupCommon()

android {
    namespace = "com.topjohnwu.magisk.wear"

    buildFeatures {
        compose = true
    }

    compileOptions {
        isCoreLibraryDesugaringEnabled = true
    }
}

dependencies {
    implementation(project(":core"))
    implementation(project(":shared"))
    coreLibraryDesugaring(libs.jdk.libs)

    // Compose
    implementation(libs.compose.ui)
    implementation(libs.compose.foundation)
    implementation(libs.compose.material.icons.extended)
    implementation(libs.activity.compose)
    implementation(libs.lifecycle.runtime.compose)
    implementation(libs.lifecycle.viewmodel.compose)

    // Wear OS
    implementation(libs.wear.compose.foundation)
    implementation(libs.wear.compose.material3)

    testImplementation(libs.junit)
}
