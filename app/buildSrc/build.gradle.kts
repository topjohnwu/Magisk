plugins {
    `kotlin-dsl`
}

repositories {
    google()
    mavenCentral()
}

gradlePlugin {
    plugins {
        register("MagiskPlugin") {
            id = "MagiskPlugin"
            implementationClass = "MagiskPlugin"
        }
    }
}

dependencies {
    implementation(kotlin("gradle-plugin", libs.versions.kotlin.get()))
    implementation("org.jetbrains.kotlin:compose-compiler-gradle-plugin:${libs.versions.kotlin.get()}")
    implementation("org.jetbrains.kotlin:kotlin-serialization:${libs.versions.kotlin.get()}")
    implementation(libs.android.gradle.plugin)
}
