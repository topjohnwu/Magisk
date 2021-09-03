plugins {
    id("com.android.library")
}

android {
    defaultConfig {
        vectorDrawables.useSupportLibrary = true
        consumerProguardFiles("proguard-rules.pro")
    }
}

dependencies {
    api("io.michaelrocks:paranoid-core:0.3.5")
}
