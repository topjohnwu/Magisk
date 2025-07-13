
import org.eclipse.jgit.internal.storage.file.FileRepository
import org.gradle.api.Plugin
import org.gradle.api.Project
import org.gradle.kotlin.dsl.provideDelegate
import java.io.File
import java.util.Properties
import java.util.Random

// Set non-zero value here to fix the random seed for reproducible builds
// CI builds are always reproducible
val RAND_SEED = if (System.getenv("CI") != null) 42 else 0
lateinit var RANDOM: Random

private val props = Properties()
private var commitHash = ""
private val supportAbis = setOf("armeabi-v7a", "x86", "arm64-v8a", "x86_64", "riscv64")
private val defaultAbis = setOf("armeabi-v7a", "x86", "arm64-v8a", "x86_64")

object Config {
    operator fun get(key: String): String? {
        val v = props[key] as? String ?: return null
        return v.ifBlank { null }
    }

    fun contains(key: String) = get(key) != null

    val version: String get() = get("version") ?: commitHash
    val versionCode: Int get() = get("magisk.versionCode")!!.toInt()
    val stubVersion: String get() = get("magisk.stubVersion")!!
    val abiList: Set<String> get() {
        val abiList = get("abiList") ?: return defaultAbis
        return abiList.split(Regex("\\s*,\\s*")).toSet() intersect supportAbis
    }
}

fun Project.rootFile(path: String): File {
    val file = File(path)
    return if (file.isAbsolute) file
    else File(rootProject.file(".."), path)
}

class MagiskPlugin : Plugin<Project> {
    override fun apply(project: Project) = project.applyPlugin()

    private fun Project.applyPlugin() {
        initRandom(rootProject.file("dict.txt"))
        props.clear()
        rootProject.file("gradle.properties").inputStream().use { props.load(it) }
        val configPath: String? by this
        val config = rootFile(configPath ?: "config.prop")
        if (config.exists())
            config.inputStream().use { props.load(it) }

        val repo = FileRepository(rootFile(".git"))
        val refId = repo.refDatabase.exactRef("HEAD").objectId
        commitHash = repo.newObjectReader().abbreviate(refId, 8).name()
    }
}
