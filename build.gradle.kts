plugins {
    id("MagiskPlugin")
}

tasks.register("clean", Delete::class) {
    delete(rootProject.buildDir, "${rootProject.buildDir}/out", "${rootProject.buildDir}/native/out")
}
