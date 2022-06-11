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

tasks.withType<KotlinCompile> {
    kotlinOptions {
        jvmTarget = "11"
    }
}

dependencies {
    implementation(kotlin("gradle-plugin", "1.7.0"))
    implementation("com.android.tools.build:gradle:7.2.1")
    implementation("androidx.navigation:navigation-safe-args-gradle-plugin:2.5.0-rc01")
    implementation("io.michaelrocks:paranoid-gradle-plugin:0.3.7")
    implementation("org.eclipse.jgit:org.eclipse.jgit:6.1.0.202203080745-r")
}
