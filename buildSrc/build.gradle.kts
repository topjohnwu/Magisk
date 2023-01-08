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
    implementation(kotlin("gradle-plugin", "1.8.0"))
    implementation("com.android.tools.build:gradle:7.3.1")
    implementation("androidx.navigation:navigation-safe-args-gradle-plugin:2.5.3")
    implementation("io.michaelrocks:paranoid-gradle-plugin:0.3.7")
    implementation("org.eclipse.jgit:org.eclipse.jgit:6.4.0.202211300538-r")
}
