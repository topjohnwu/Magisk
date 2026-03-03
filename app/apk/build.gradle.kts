plugins {
    id("com.android.application")
    kotlin("plugin.parcelize")
    kotlin("plugin.compose")
    kotlin("plugin.serialization")
    id("com.android.legacy-kapt")
}

setupMainApk()

kapt {
    correctErrorTypes = true
    useBuildCache = true
    mapDiagnosticLocations = true
    javacOptions {
        option("-Xmaxerrs", "1000")
    }
}

android {
    buildFeatures {
        dataBinding = true
        compose = true
    }

    compileOptions {
        isCoreLibraryDesugaringEnabled = true
    }

    defaultConfig {
        proguardFile("proguard-rules.pro")
    }

    buildTypes {
        release {
            isMinifyEnabled = true
            isShrinkResources = true
        }
    }
}

dependencies {
    implementation(project(":core"))
    coreLibraryDesugaring(libs.jdk.libs)

    implementation(libs.rikka.layoutinflater)
    implementation(libs.rikka.insets)

    implementation(libs.constraintlayout)
    implementation(libs.recyclerview)
    implementation(libs.appcompat)
    implementation(libs.material)

    // Compose
    implementation(platform(libs.compose.bom))
    implementation(libs.compose.ui)
    implementation(libs.compose.ui.tooling.preview)
    debugImplementation(libs.compose.ui.tooling)
    implementation(libs.activity.compose)
    implementation(libs.lifecycle.runtime.compose)
    implementation(libs.lifecycle.viewmodel.compose)
    implementation(libs.miuix)
    implementation(libs.miuix.icons)
    implementation(libs.miuix.navigation3.ui)

    // Navigation3
    implementation(libs.navigation3.runtime)
    implementation(libs.navigationevent.compose)
    implementation(libs.lifecycle.viewmodel.navigation3)

    // Make sure kapt runs with a proper kotlin-stdlib
    kapt(kotlin("stdlib"))
}
