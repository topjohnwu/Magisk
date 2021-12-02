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
    implementation(kotlin("gradle-plugin", "1.5.31"))
    implementation("com.android.tools.build:gradle:7.0.3")
    implementation("androidx.navigation:navigation-safe-args-gradle-plugin:2.4.0-alpha10")
    implementation("io.michaelrocks:paranoid-gradle-plugin:0.3.5")
    implementation("org.eclipse.jgit:org.eclipse.jgit:5.12.0.202106070339-r")
}
