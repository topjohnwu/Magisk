plugins {
    alias(libs.plugins.android.library)
    kotlin("plugin.parcelize")
    alias(libs.plugins.compose.compiler)
}

setupCommon()

android {
    namespace = "com.topjohnwu.magisk.shared"

    compileOptions {
        isCoreLibraryDesugaringEnabled = true
    }
}

dependencies {
    api(project(":core"))
    coreLibraryDesugaring(libs.jdk.libs)

    api(libs.lifecycle.viewmodel)
    api(libs.lifecycle.viewmodel.savedstate)
    api(libs.lifecycle.livedata.core)
    api(libs.compose.runtime)
    api(libs.navigation3.runtime)
    implementation(libs.core.ktx)
}
