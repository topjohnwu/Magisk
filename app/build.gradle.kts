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

    dependenciesInfo {
        includeInApk = false
        includeInBundle = false
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
        }
        jniLibs {
            keepDebugSymbols += "**/*.so"
        }
    }

    kotlinOptions {
        jvmTarget = "11"
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

    val vBC = "1.69"
    implementation("org.bouncycastle:bcprov-jdk15on:${vBC}")
    implementation("org.bouncycastle:bcpkix-jdk15on:${vBC}")

    val vBAdapt = "4.0.0"
    val bindingAdapter = "me.tatarka.bindingcollectionadapter2:bindingcollectionadapter"
    implementation("${bindingAdapter}:${vBAdapt}")
    implementation("${bindingAdapter}-recyclerview:${vBAdapt}")

    val vMarkwon = "4.6.2"
    implementation("io.noties.markwon:core:${vMarkwon}")
    implementation("io.noties.markwon:html:${vMarkwon}")
    implementation("io.noties.markwon:image:${vMarkwon}")
    implementation("com.caverock:androidsvg:1.4")

    val vLibsu = "3.1.2"
    implementation("com.github.topjohnwu.libsu:core:${vLibsu}")
    implementation("com.github.topjohnwu.libsu:io:${vLibsu}")

    val vRetrofit = "2.9.0"
    implementation("com.squareup.retrofit2:retrofit:${vRetrofit}")
    implementation("com.squareup.retrofit2:converter-moshi:${vRetrofit}")
    implementation("com.squareup.retrofit2:converter-scalars:${vRetrofit}")

    val vOkHttp = "4.9.2"
    implementation("com.squareup.okhttp3:okhttp:${vOkHttp}")
    implementation("com.squareup.okhttp3:logging-interceptor:${vOkHttp}")

    val vMoshi = "1.12.0"
    implementation("com.squareup.moshi:moshi:${vMoshi}")
    kapt("com.squareup.moshi:moshi-kotlin-codegen:${vMoshi}")

    val vRoom = "2.3.0"
    implementation("androidx.room:room-runtime:${vRoom}")
    implementation("androidx.room:room-ktx:${vRoom}")
    kapt("androidx.room:room-compiler:${vRoom}")

    val vNav = "2.4.0-alpha10"
    implementation("androidx.navigation:navigation-fragment-ktx:${vNav}")
    implementation("androidx.navigation:navigation-ui-ktx:${vNav}")

    implementation("androidx.biometric:biometric:1.1.0")
    implementation("androidx.constraintlayout:constraintlayout:2.1.1")
    implementation("androidx.swiperefreshlayout:swiperefreshlayout:1.1.0")
    implementation("androidx.browser:browser:1.4.0")
    implementation("androidx.preference:preference:1.1.1")
    implementation("androidx.recyclerview:recyclerview:1.2.1")
    implementation("androidx.fragment:fragment-ktx:1.3.6")
    implementation("androidx.transition:transition:1.4.1")
    implementation("androidx.core:core-ktx:1.7.0")
    implementation("androidx.core:core-splashscreen:1.0.0-alpha02")
    implementation("com.google.android.material:material:1.4.0")
}
