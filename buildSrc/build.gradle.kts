plugins {
    `kotlin-dsl`
}
repositories {
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
    implementation("org.eclipse.jgit:org.eclipse.jgit:5.12.0.202106070339-r")
}
