
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
    operator fun get(key: String) : String? {
        val v = props[key] as? String ?: return null
        return if (v.isBlank()) null else v
    }
    fun contains(key: String) = get(key) != null

    val appVersion: String get() = get("appVersion") ?: commitHash
    val appVersionCode: Int get() = get("appVersionCode")?.toInt() ?: commitCount
    val magiskVersionCode: Int get() = get("versionCode")?.toInt() ?: Int.MAX_VALUE
}

class MagiskPlugin : Plugin<Project> {
    override fun apply(project: Project) {
        val configPath: String? by project
        val config = configPath?.let { File(it) } ?: project.file("config.prop")
        if (config.exists())
            config.inputStream().use { props.load(it) }

        if (!Config.contains("appVersion") || !Config.contains("appVersionCode")) {
            val repo = FileRepository(project.rootProject.file(".git"))
            val refId = repo.refDatabase.exactRef("refs/heads/master").objectId
            commitHash = repo.newObjectReader().abbreviate(refId, 8).name()
            commitCount = Git(repo).log().add(refId).call().count()
        }
    }
}
