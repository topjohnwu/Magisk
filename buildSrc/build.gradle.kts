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
    implementation(kotlin("gradle-plugin", "1.6.10"))
    implementation("com.android.tools.build:gradle:7.0.4")
    implementation("androidx.navigation:navigation-safe-args-gradle-plugin:2.4.0-rc01")
    implementation("io.michaelrocks:paranoid-gradle-plugin:0.3.7")
    implementation("org.eclipse.jgit:org.eclipse.jgit:5.12.0.202106070339-r")
}
