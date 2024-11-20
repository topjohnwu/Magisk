plugins {
    id("com.android.application")
    id("org.lsposed.lsparanoid")
}

lsparanoid {
    seed = if (RAND_SEED != 0) RAND_SEED else null
    includeDependencies = true
    classFilter = { true }
}

android {
    namespace = "com.topjohnwu.magisk"

    val canary = !Config.version.contains(".")

    val url = if (canary) null
    else "https://github.com/topjohnwu/Magisk/releases/download/v${Config.version}/Magisk-v${Config.version}.apk"

    defaultConfig {
        applicationId = "com.topjohnwu.magisk"
        versionCode = 1
        versionName = "1.0"
        buildConfigField("String", "APK_URL", url?.let { "\"$it\"" } ?: "null" )
        buildConfigField("int", "STUB_VERSION", Config.stubVersion)
    }

    buildTypes {
        release {
            proguardFiles("proguard-rules.pro")
            isMinifyEnabled = true
            isShrinkResources = false
        }
    }

    buildFeatures {
        buildConfig = true
    }
}

setupStubApk()

dependencies {
    implementation(project(":app:shared"))
}
