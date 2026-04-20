plugins {
    alias(libs.plugins.android.library)
}

setupCommon()

android {
    namespace = "com.topjohnwu.shared"
    enableKotlin = false
}
