plugins {
    id("com.android.library")
    kotlin("android")
    kotlin("plugin.parcelize")
    id("com.google.devtools.ksp")
}

setupCoreLib()

ksp {
    arg("room.generateKotlin", "true")
}

android {
    namespace = "com.topjohnwu.magisk.core"

    defaultConfig {
        buildConfigField("String", "APP_PACKAGE_NAME", "\"com.topjohnwu.magisk\"")
        buildConfigField("int", "APP_VERSION_CODE", "${Config.versionCode}")
        buildConfigField("String", "APP_VERSION_NAME", "\"${Config.version}\"")
        buildConfigField("int", "STUB_VERSION", Config.stubVersion)
    }

    buildFeatures {
        aidl = true
        buildConfig = true
    }
}

dependencies {
    api(project(":app:shared"))

    api(libs.timber)
    api(libs.markwon.core)
    implementation(libs.bcpkix)
    implementation(libs.commons.compress)

    api(libs.libsu.core)
    api(libs.libsu.service)
    api(libs.libsu.nio)

    implementation(libs.retrofit)
    implementation(libs.retrofit.moshi)
    implementation(libs.retrofit.scalars)

    implementation(libs.okhttp)
    implementation(libs.okhttp.logging)
    implementation(libs.okhttp.dnsoverhttps)

    implementation(libs.moshi)
    ksp(libs.moshi.codegen)

    implementation(libs.room.runtime)
    implementation(libs.room.ktx)
    ksp(libs.room.compiler)

    implementation(libs.core.splashscreen)
    implementation(libs.core.ktx)
    implementation(libs.activity)
    implementation(libs.collection.ktx)
    implementation(libs.profileinstaller)
    implementation(libs.lifecycle.process)
}
