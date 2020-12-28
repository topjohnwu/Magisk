plugins {
    id("com.android.application")
}

android {
    val canary = !Config["appVersion"].orEmpty().contains(".")

    defaultConfig {
        applicationId = "com.topjohnwu.magisk"
        versionCode = 1
        versionName = Config.appVersion
        buildConfigField("String", "DEV_CHANNEL", Config["DEV_CHANNEL"] ?: "null")
        buildConfigField("boolean", "CANARY", if (canary) "true" else "false")
    }

    buildTypes {
        getByName("release") {
            isMinifyEnabled = true
            isShrinkResources = true
            proguardFiles("proguard-rules.pro")
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
