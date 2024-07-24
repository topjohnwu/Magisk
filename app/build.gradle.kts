tasks.register("clean") {
    subprojects.forEach {
        dependsOn(":app:${it.name}:clean")
    }
}
