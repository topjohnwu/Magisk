plugins {
    id("com.android.application")
}

android {
    defaultConfig {
        applicationId = "com.topjohnwu.magisk"
        versionCode = 1
        versionName = Config["appVersion"]
        buildConfigField("String", "DEV_CHANNEL", Config["DEV_CHANNEL"] ?: "null")
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

    dependenciesInfo {
        includeInApk = false
        includeInBundle = false
    }
}

dependencies {
    implementation(fileTree(mapOf("dir" to "libs", "include" to listOf("*.jar"))))
    implementation(project(":app:shared"))
}
