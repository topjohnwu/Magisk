plugins {
    alias(libs.plugins.android.application)
}

android {
    namespace = "com.topjohnwu.magisk"
    enableKotlin = false

    val base = "https://github.com/topjohnwu/Magisk/releases/download/"
    val url = base + "v${Config.version}/Magisk-v${Config.version}.apk"

    defaultConfig {
        applicationId = "com.topjohnwu.magisk"
        versionCode = 1
        versionName = "1.0"
        buildConfigField("String", "APK_URL", "\"$url\"")
    }

    buildTypes {
        release {
            proguardFiles("proguard-rules.pro")
            isMinifyEnabled = true
        }
    }

    buildFeatures {
        buildConfig = true
    }
}

setupAppCommon()

dependencies {
    implementation(project(":shared"))
}
