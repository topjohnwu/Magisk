plugins {
    id("com.android.application")
    kotlin("android")
    kotlin("plugin.parcelize")
    kotlin("kapt")
    id("androidx.navigation.safeargs.kotlin")
}

kapt {
    correctErrorTypes = true
    useBuildCache = true
    mapDiagnosticLocations = true
    javacOptions {
        option("-Xmaxerrs", 1000)
    }
    arguments {
        arg("room.incremental", "true")
    }
}

android {
    defaultConfig {
        applicationId = "com.topjohnwu.magisk"
        vectorDrawables.useSupportLibrary = true
        versionName = Config.version
        versionCode = Config.versionCode
        ndk.abiFilters += listOf("armeabi-v7a", "arm64-v8a", "x86", "x86_64")
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

    packagingOptions {
        resources {
            excludes += "/META-INF/*"
            excludes += "/org/bouncycastle/**"
            excludes += "/kotlin/**"
            excludes += "/kotlinx/**"
            excludes += "/okhttp3/**"
            excludes += "/*.txt"
            excludes += "/*.bin"
            excludes += "/*.json"
        }
        jniLibs {
            keepDebugSymbols += "**/*.so"
        }
    }

    kotlinOptions {
        jvmTarget = "11"
        freeCompilerArgs = listOf("-Xjvm-default=enable")
    }
}

setupApp()

configurations.all {
    exclude("org.jetbrains.kotlin", "kotlin-stdlib-jdk7")
    exclude("org.jetbrains.kotlin", "kotlin-stdlib-jdk8")
}

dependencies {
    implementation(project(":app:shared"))

    implementation("com.github.topjohnwu:jtar:1.0.0")
    implementation("com.github.topjohnwu:indeterminate-checkbox:1.0.7")
    implementation("com.github.topjohnwu:lz4-java:1.7.1")
    implementation("com.jakewharton.timber:timber:4.7.1")
    implementation("org.bouncycastle:bcpkix-jdk15on:1.70")
    implementation("dev.rikka.rikkax.layoutinflater:layoutinflater:1.2.0")
    implementation("dev.rikka.rikkax.insets:insets:1.1.1")
    implementation("dev.rikka.rikkax.recyclerview:recyclerview-ktx:1.3.1")
    implementation("io.noties.markwon:core:4.6.2")

    val vBAdapt = "4.0.0"
    val bindingAdapter = "me.tatarka.bindingcollectionadapter2:bindingcollectionadapter"
    implementation("${bindingAdapter}:${vBAdapt}")
    implementation("${bindingAdapter}-recyclerview:${vBAdapt}")

    val vLibsu = "3.2.1"
    implementation("com.github.topjohnwu.libsu:core:${vLibsu}")
    implementation("com.github.topjohnwu.libsu:io:${vLibsu}")
    implementation("com.github.topjohnwu.libsu:service:${vLibsu}")

    val vRetrofit = "2.9.0"
    implementation("com.squareup.retrofit2:retrofit:${vRetrofit}")
    implementation("com.squareup.retrofit2:converter-moshi:${vRetrofit}")
    implementation("com.squareup.retrofit2:converter-scalars:${vRetrofit}")

    val vOkHttp = "4.9.3"
    implementation("com.squareup.okhttp3:okhttp:${vOkHttp}")
    implementation("com.squareup.okhttp3:logging-interceptor:${vOkHttp}")
    implementation("com.squareup.okhttp3:okhttp-dnsoverhttps:${vOkHttp}")

    val vMoshi = "1.13.0"
    implementation("com.squareup.moshi:moshi:${vMoshi}")
    kapt("com.squareup.moshi:moshi-kotlin-codegen:${vMoshi}")

    val vRoom = "2.4.1"
    implementation("androidx.room:room-runtime:${vRoom}")
    implementation("androidx.room:room-ktx:${vRoom}")
    kapt("androidx.room:room-compiler:${vRoom}")

    val vNav = "2.5.0-alpha01"
    implementation("androidx.navigation:navigation-fragment-ktx:${vNav}")
    implementation("androidx.navigation:navigation-ui-ktx:${vNav}")

    implementation("androidx.biometric:biometric:1.1.0")
    implementation("androidx.constraintlayout:constraintlayout:2.1.3")
    implementation("androidx.swiperefreshlayout:swiperefreshlayout:1.1.0")
    implementation("androidx.appcompat:appcompat:1.4.1")
    implementation("androidx.preference:preference:1.2.0")
    implementation("androidx.recyclerview:recyclerview:1.2.1")
    implementation("androidx.fragment:fragment-ktx:1.4.1")
    implementation("androidx.transition:transition:1.4.1")
    implementation("androidx.core:core-ktx:1.7.0")
    implementation("androidx.core:core-splashscreen:1.0.0-beta01")
    implementation("com.google.android.material:material:1.5.0")
}
