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

    api("com.jakewharton.timber:timber:5.0.1")
    api("io.noties.markwon:core:4.6.2")
    implementation("org.bouncycastle:bcpkix-jdk18on:1.78.1")
    implementation("org.apache.commons:commons-compress:1.26.2")

    val vLibsu = "6.0.0"
    api("com.github.topjohnwu.libsu:core:${vLibsu}")
    api("com.github.topjohnwu.libsu:service:${vLibsu}")
    api("com.github.topjohnwu.libsu:nio:${vLibsu}")

    val vRetrofit = "2.11.0"
    implementation("com.squareup.retrofit2:retrofit:${vRetrofit}")
    implementation("com.squareup.retrofit2:converter-moshi:${vRetrofit}")
    implementation("com.squareup.retrofit2:converter-scalars:${vRetrofit}")

    val vOkHttp = "4.12.0"
    implementation("com.squareup.okhttp3:okhttp:${vOkHttp}")
    implementation("com.squareup.okhttp3:logging-interceptor:${vOkHttp}")
    implementation("com.squareup.okhttp3:okhttp-dnsoverhttps:${vOkHttp}")

    val vMoshi = "1.15.1"
    implementation("com.squareup.moshi:moshi:${vMoshi}")
    ksp("com.squareup.moshi:moshi-kotlin-codegen:${vMoshi}")

    val vRoom = "2.6.1"
    implementation("androidx.room:room-runtime:${vRoom}")
    implementation("androidx.room:room-ktx:${vRoom}")
    ksp("androidx.room:room-compiler:${vRoom}")

    implementation("androidx.core:core-splashscreen:1.0.1")
    implementation("androidx.core:core-ktx:1.13.1")
    implementation("androidx.activity:activity:1.9.0")
    implementation("androidx.collection:collection-ktx:1.4.1")
    implementation("androidx.profileinstaller:profileinstaller:1.3.1")
    implementation("androidx.lifecycle:lifecycle-process:2.8.3")
}
