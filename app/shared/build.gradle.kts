plugins {
    id("com.android.library")
}

setupCommon()

android {
    defaultConfig {
        consumerProguardFiles("proguard-rules.pro")
    }
}

dependencies {
    api("io.michaelrocks:paranoid-core:0.3.7")
}
