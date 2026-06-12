plugins {
    alias(libs.plugins.android.application)
}

setupCommon()

android {
    namespace = "com.topjohnwu.magisk"
    enableKotlin = false

    buildTypes {
        release {
            isShrinkResources = false
        }
    }
}
