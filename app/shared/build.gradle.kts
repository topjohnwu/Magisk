val kotlin_version: String by extra
plugins {
    id("com.android.library")
    kotlin("android")
    kotlin("plugin.parcelize")
    kotlin("kapt")
    id("androidx.navigation.safeargs.kotlin")
}
apply {
    plugin("kotlin-android")
}

android {
    defaultConfig {
        vectorDrawables.useSupportLibrary = true
        consumerProguardFiles("proguard-rules.pro")
    }

    packagingOptions {
        exclude("/META-INF/**")
        exclude("/org/bouncycastle/**")
        exclude("/kotlin/**")
        exclude("/kotlinx/**")
        exclude("/okhttp3/**")
        exclude("/*.txt")
        exclude("/*.bin")
    }

    kotlinOptions {
        jvmTarget = "1.8"
    }
}

dependencies {
    implementation(fileTree(mapOf("dir" to "libs", "include" to listOf("*.jar"))))
    implementation("androidx.core:core-ktx:+")

    implementation(kotlin("stdlib"))
}
repositories {
    mavenCentral()
}