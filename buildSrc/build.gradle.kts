import org.jetbrains.kotlin.gradle.tasks.KotlinCompile

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

tasks.withType<KotlinCompile>().configureEach {
    kotlinOptions {
        languageVersion = "2.0"
    }
}

dependencies {
    implementation(kotlin("gradle-plugin", "2.0.0"))
    implementation("com.android.tools.build:gradle:8.5.1")
    implementation("com.google.devtools.ksp:com.google.devtools.ksp.gradle.plugin:2.0.0-1.0.23")
    implementation("androidx.navigation:navigation-safe-args-gradle-plugin:2.7.7")
    implementation("org.lsposed.lsparanoid:gradle-plugin:0.5.2")
    implementation("org.eclipse.jgit:org.eclipse.jgit:6.10.0.202406032230-r")
}
