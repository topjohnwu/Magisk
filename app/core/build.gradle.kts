plugins {
    id("com.android.library")
    kotlin("android")
    kotlin("plugin.parcelize")
    id("dev.zacsweers.moshix")
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
        consumerProguardFile("proguard-rules.pro")
    }

    buildFeatures {
        aidl = true
        buildConfig = true
    }

    compileOptions {
        isCoreLibraryDesugaringEnabled = true
    }
}

dependencies {
    api(project(":shared"))
    coreLibraryDesugaring(libs.jdk.libs)

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

    implementation(libs.room.runtime)
    implementation(libs.room.ktx)
    ksp(libs.room.compiler)

    implementation(libs.core.splashscreen)
    implementation(libs.core.ktx)
    implementation(libs.activity)
    implementation(libs.collection.ktx)
    implementation(libs.profileinstaller)

    // We also implement all our tests in this module.
    // However, we don't want to bundle test dependencies.
    // That's why we make it compileOnly.
    compileOnly(libs.test.junit)
    compileOnly(libs.test.uiautomator)
}
