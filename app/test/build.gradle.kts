plugins {
    id("com.android.application")
    kotlin("android")
}

android {
    namespace = "com.topjohnwu.magisk.test"

    defaultConfig {
        applicationId = "com.topjohnwu.magisk.test"
        versionCode = 1
        versionName = "1.0"
    }
}

setupAppCommon()

dependencies {
    compileOnly(project(":app:core"))

    implementation(libs.test.runner)
    implementation(libs.test.rules)
    implementation(libs.test.junit)
}
