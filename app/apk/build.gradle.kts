plugins {
    id("com.android.application")
    kotlin("android")
    kotlin("plugin.parcelize")
    kotlin("kapt")
    id("androidx.navigation.safeargs.kotlin")
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
    }

    compileOptions {
        isCoreLibraryDesugaringEnabled = true
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

    implementation(libs.indeterminate.checkbox)
    implementation(libs.rikka.layoutinflater)
    implementation(libs.rikka.insets)
    implementation(libs.rikka.recyclerview)

    implementation(libs.navigation.fragment.ktx)
    implementation(libs.navigation.ui.ktx)

    implementation(libs.constraintlayout)
    implementation(libs.swiperefreshlayout)
    implementation(libs.recyclerview)
    implementation(libs.transition)
    implementation(libs.fragment.ktx)
    implementation(libs.appcompat)
    implementation(libs.material)

    // Make sure kapt runs with a proper kotlin-stdlib
    kapt(kotlin("stdlib"))
}
