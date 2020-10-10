import com.github.jengelman.gradle.plugins.shadow.tasks.ShadowJar

plugins {
    id("java-library")
    id("java")
    id("com.github.johnrengelman.shadow") version "6.0.0"
}

java {
    sourceCompatibility = JavaVersion.VERSION_1_8
    targetCompatibility = JavaVersion.VERSION_1_8
}

val jar by tasks.getting(Jar::class) {
    manifest {
        attributes["Main-Class"] = "com.topjohnwu.signing.ZipSigner"
    }
}

val shadowJar by tasks.getting(ShadowJar::class) {
    archiveBaseName.set("zipsigner")
    archiveClassifier.set(null as String?)
    archiveVersion.set("4.0")
}

repositories {
    jcenter()
}

dependencies {
    implementation(fileTree(mapOf("dir" to "libs", "include" to listOf("*.jar"))))

    api("org.bouncycastle:bcprov-jdk15on:1.66")
    api("org.bouncycastle:bcpkix-jdk15on:1.66")
}
