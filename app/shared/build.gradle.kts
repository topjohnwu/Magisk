plugins {
    id("com.android.library")
}

android {
    defaultConfig {
        vectorDrawables.useSupportLibrary = true
        consumerProguardFiles("proguard-rules.pro")
    }
}
