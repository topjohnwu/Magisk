import org.jetbrains.kotlin.gradle.dsl.KotlinVersion

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
    implementation(libs.android.gradle.plugin)
    implementation(libs.ksp.plugin)
    implementation(libs.navigation.safe.args.plugin)
    implementation(libs.lsparanoid.plugin)
    implementation(libs.moshi.plugin)
    implementation(libs.jgit)
    implementation(libs.protobuf.plugin)
}
