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
    implementation(kotlin("gradle-plugin", "1.6.10"))
    implementation("com.android.tools.build:gradle:7.1.1")
    implementation("androidx.navigation:navigation-safe-args-gradle-plugin:2.5.0-alpha01")
    implementation("io.michaelrocks:paranoid-gradle-plugin:0.3.7")
    implementation("org.eclipse.jgit:org.eclipse.jgit:5.12.0.202106070339-r")
}
