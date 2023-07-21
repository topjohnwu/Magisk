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
    // Cannot upgrade to 1.9.0: https://issuetracker.google.com/issues/236612358#comment19
    implementation(kotlin("gradle-plugin", "1.8.22"))
    implementation("com.android.tools.build:gradle:8.0.2")
    implementation("androidx.navigation:navigation-safe-args-gradle-plugin:2.6.0")
    implementation("org.lsposed.lsparanoid:gradle-plugin:0.5.2")
    implementation("org.eclipse.jgit:org.eclipse.jgit:6.5.0.202303070854-r")
}
