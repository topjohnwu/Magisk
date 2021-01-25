import org.eclipse.jgit.api.Git
import org.eclipse.jgit.internal.storage.file.FileRepository
import org.gradle.api.Plugin
import org.gradle.api.Project
import org.gradle.kotlin.dsl.provideDelegate
import java.io.File
import java.util.*

private val props = Properties()
private lateinit var commitHash: String
private var commitCount = 0

object Config {
    operator fun get(key: String): String? {
        val v = props[key] as? String ?: return null
        return if (v.isBlank()) null else v
    }

    fun contains(key: String) = get(key) != null

    val version: String get() = get("version") ?: commitHash
    val versionCode: Int get() = get("magisk.versionCode")!!.toInt()
    val stubVersion: String get() = get("magisk.stubVersion")!!
}

class MagiskPlugin : Plugin<Project> {
    override fun apply(project: Project) {
        project.rootProject.file("gradle.properties").inputStream().use { props.load(it) }
        val configPath: String? by project
        val config = configPath?.let { File(it) } ?: project.rootProject.file("config.prop")
        if (config.exists())
            config.inputStream().use { props.load(it) }

        val repo = FileRepository(project.rootProject.file(".git"))
        val refId = repo.refDatabase.exactRef("HEAD").objectId
        commitHash = repo.newObjectReader().abbreviate(refId, 8).name()
        commitCount = Git(repo).log().add(refId).call().count()
    }
}
