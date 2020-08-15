plugins {
    id("com.android.application")
    kotlin("android")
    kotlin("android.extensions")
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
}

android {
    defaultConfig {
        applicationId = "com.topjohnwu.magisk"
        vectorDrawables.useSupportLibrary = true
        multiDexEnabled = true
        versionName = Config["appVersion"]
        versionCode = Config["appVersionCode"]?.toInt()
        buildConfigField("int", "LATEST_MAGISK", Config["versionCode"] ?: "Integer.MAX_VALUE")

        javaCompileOptions.annotationProcessorOptions.arguments(
                mapOf("room.incremental" to "true")
        )
    }

    buildTypes {
        getByName("release") {
            isMinifyEnabled = true
            isShrinkResources = true
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
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
        exclude("/META-INF/**")
        exclude("/androidsupportmultidexversion.txt")
        exclude("/org/bouncycastle/**")
        exclude("/kotlin/**")
        exclude("/kotlinx/**")
        exclude("/okhttp3/**")
    }

    kotlinOptions {
        jvmTarget = "1.8"
    }
}

androidExtensions {
    isExperimental = true
}

val copyUtils = tasks.register("copyUtils", Copy::class) {
    from(rootProject.file("scripts/util_functions.sh"))
    into("src/main/res/raw")
}

tasks["preBuild"]?.dependsOn(copyUtils)

dependencies {
    implementation(fileTree(mapOf("dir" to "libs", "include" to listOf("*.jar"))))
    implementation(kotlin("stdlib"))
    implementation(project(":app:shared"))
    implementation(project(":app:signing"))

    implementation("com.github.topjohnwu:jtar:1.0.0")
    implementation("com.github.topjohnwu:indeterminate-checkbox:1.0.7")
    implementation("com.jakewharton.timber:timber:4.7.1")

    val vBAdapt = "4.0.0"
    val bindingAdapter = "me.tatarka.bindingcollectionadapter2:bindingcollectionadapter"
    implementation("${bindingAdapter}:${vBAdapt}")
    implementation("${bindingAdapter}-recyclerview:${vBAdapt}")

    val vMarkwon = "4.5.1"
    implementation("io.noties.markwon:core:${vMarkwon}")
    implementation("io.noties.markwon:html:${vMarkwon}")
    implementation("io.noties.markwon:image:${vMarkwon}")
    implementation("com.caverock:androidsvg:1.4")

    val vLibsu = "3.0.2"
    implementation("com.github.topjohnwu.libsu:core:${vLibsu}")
    implementation("com.github.topjohnwu.libsu:io:${vLibsu}")

    val vKoin = "2.1.6"
    implementation("org.koin:koin-core:${vKoin}")
    implementation("org.koin:koin-android:${vKoin}")
    implementation("org.koin:koin-androidx-viewmodel:${vKoin}")

    val vRetrofit = "2.9.0"
    implementation("com.squareup.retrofit2:retrofit:${vRetrofit}")
    implementation("com.squareup.retrofit2:converter-moshi:${vRetrofit}")
    implementation("com.squareup.retrofit2:converter-scalars:${vRetrofit}")

    val vOkHttp = "3.12.12"
    implementation("com.squareup.okhttp3:okhttp") {
        version {
            strictly(vOkHttp)
        }
    }
    implementation("com.squareup.okhttp3:logging-interceptor:${vOkHttp}")
    implementation("com.squareup.okhttp3:okhttp-dnsoverhttps:${vOkHttp}")

    val vMoshi = "1.9.3"
    implementation("com.squareup.moshi:moshi:${vMoshi}")
    kapt("com.squareup.moshi:moshi-kotlin-codegen:${vMoshi}")

    val vRoom = "2.2.5"
    implementation("androidx.room:room-runtime:${vRoom}")
    implementation("androidx.room:room-ktx:${vRoom}")
    kapt("androidx.room:room-compiler:${vRoom}")

    val vNav: String by rootProject.extra
    implementation("androidx.navigation:navigation-fragment-ktx:${vNav}")
    implementation("androidx.navigation:navigation-ui-ktx:${vNav}")

    implementation("androidx.biometric:biometric:1.0.1")
    implementation("androidx.constraintlayout:constraintlayout:2.0.0-rc1")
    implementation("androidx.swiperefreshlayout:swiperefreshlayout:1.1.0")
    implementation("androidx.browser:browser:1.2.0")
    implementation("androidx.preference:preference:1.1.1")
    implementation("androidx.recyclerview:recyclerview:1.1.0")
    implementation("androidx.fragment:fragment-ktx:1.2.5")
    implementation("androidx.work:work-runtime-ktx:2.4.0")
    implementation("androidx.transition:transition:1.3.1")
    implementation("androidx.multidex:multidex:2.0.1")
    implementation("androidx.core:core-ktx:1.3.1")
    implementation("androidx.localbroadcastmanager:localbroadcastmanager:1.0.0")
    implementation("com.google.android.material:material:1.2.0")
}
